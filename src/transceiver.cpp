//! \file transceiver.cpp Defines member functions for Fastcgipp_m0sh::Transceiver
/***************************************************************************
* Copyright (C) 2007 Eddie                                                 *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/


#include <fastcgipp-mosh/transceiver.hpp>

int Fastcgipp_m0sh::Transceiver::transmit()
{
	while (1) {{
			Buffer::SendBlock sendBlock(buffer.requestRead());
			if (sendBlock.size) {
				ssize_t sent = write(sendBlock.fd, sendBlock.data, sendBlock.size);
				if (sent < 0) {
					if (errno == EPIPE) {
						pollFds.erase(std::find_if(pollFds.begin(), pollFds.end(), equalsFd(sendBlock.fd)));
						fdBuffers.erase(sendBlock.fd);
						sent = sendBlock.size;
					} else if (errno != EAGAIN) throw Exceptions::SocketWrite(sendBlock.fd, errno);
				}

				buffer.freeRead(sent);
				if (sent != sendBlock.size)
					break;
			} else
				break;
		}
	}

	return buffer.empty();
}

void Fastcgipp_m0sh::Transceiver::Buffer::secureWrite(size_t size, Protocol::FullId id, bool kill)
{
	writeIt->end += size;
	if (minBlockSize > (writeIt->data.get() + Chunk::size - writeIt->end) && ++writeIt == chunks.end()) {
		chunks.push_back(Chunk());
		--writeIt;
	}
	frames.push(Frame(size, kill, id));
}

bool Fastcgipp_m0sh::Transceiver::handler()
{
	using namespace std;
	using namespace Protocol;

	bool transmitEmpty = transmit();

	int retVal = poll(&pollFds.front(), pollFds.size(), 0);
	if (retVal == 0) {
		if (transmitEmpty) return true;
		else return false;
	}
	if (retVal < 0) throw Exceptions::Poll(errno);

	std::vector<pollfd>::iterator pollFd = find_if(pollFds.begin(), pollFds.end(), reventsZero);

	if (pollFd->revents & POLLHUP) {
		fdBuffers.erase(pollFd->fd);
		pollFds.erase(pollFd);
		return false;
	}

	int fd = pollFd->fd;
	if (fd == socket) {
		sockaddr_un addr;
		socklen_t addrlen = sizeof(sockaddr_un);
		fd = accept(fd, (sockaddr*)&addr, &addrlen);
		fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) | O_NONBLOCK) ^ O_NONBLOCK);

		pollFds.push_back(pollfd());
		pollFds.back().fd = fd;
		pollFds.back().events = POLLIN | POLLHUP;

		Message& messageBuffer = fdBuffers[fd].messageBuffer;
		messageBuffer.size = 0;
		messageBuffer.type = 0;
	} else if (fd == wakeUpFdIn) {
		char x;
		read(wakeUpFdIn, &x, 1);
		return false;
	}

	Message& messageBuffer = fdBuffers[fd].messageBuffer;
	Header& headerBuffer = fdBuffers[fd].headerBuffer;

	ssize_t actual;
	// Are we in the process of recieving some part of a frame?
	if (!messageBuffer.data) {
		// Are we recieving a partial header or new?
		actual = read(fd, (char*)&headerBuffer + messageBuffer.size, sizeof(Header) - messageBuffer.size);
		if (actual < 0 && errno != EAGAIN) throw Exceptions::SocketRead(fd, errno);
		if (actual > 0) messageBuffer.size += actual;
		if (messageBuffer.size != sizeof(Header)) {
			if (transmitEmpty) return true;
			else return false;
		}

		messageBuffer.data.reset(new char[sizeof(Header)+headerBuffer.getContentLength()+headerBuffer.getPaddingLength()]);
		memcpy(static_cast<void*>(messageBuffer.data.get()), static_cast<const void*>(&headerBuffer), sizeof(Header));
	}

	const Header& header = *(const Header*)messageBuffer.data.get();
	size_t needed = header.getContentLength() + header.getPaddingLength() + sizeof(Header) - messageBuffer.size;
	actual = read(fd, messageBuffer.data.get() + messageBuffer.size, needed);
	if (actual < 0 && errno != EAGAIN) throw Exceptions::SocketRead(fd, errno);
	if (actual > 0) messageBuffer.size += actual;

	// Did we recieve a full frame?
	if (actual == needed) {
		sendMessage(FullId(headerBuffer.getRequestId(), fd), messageBuffer);
		messageBuffer.size = 0;
		messageBuffer.data.reset();
		return false;
	}
	if (transmitEmpty) return true;
	else return false;
}

void Fastcgipp_m0sh::Transceiver::Buffer::freeRead(size_t size)
{
	pRead += size;
	if (pRead >= chunks.begin()->end) {
		if (writeIt == chunks.begin()) {
			pRead = writeIt->data.get();
			writeIt->end = pRead;
		} else {
			if (writeIt == --chunks.end()) {
				chunks.begin()->end = chunks.begin()->data.get();
				chunks.splice(chunks.end(), chunks, chunks.begin());
			} else
				chunks.pop_front();
			pRead = chunks.begin()->data.get();
		}
	}
	if ((frames.front().size -= size) == 0) {
		if (frames.front().closeFd) {
			pollFds.erase(std::find_if(pollFds.begin(), pollFds.end(), equalsFd(frames.front().id.fd)));
			close(frames.front().id.fd);
			fdBuffers.erase(frames.front().id.fd);
		}
		frames.pop();
	}

}

void Fastcgipp_m0sh::Transceiver::wake()
{
	char x;
	write(wakeUpFdOut, &x, 1);
}

Fastcgipp_m0sh::Transceiver::Transceiver(int fd_, boost::function<void(Protocol::FullId, Message)> sendMessage_)
	: sendMessage(sendMessage_), pollFds(2), socket(fd_), buffer(pollFds, fdBuffers)
{
	socket = fd_;

	// Let's setup an in/out socket for waking up poll()
	int socPair[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, socPair);
	wakeUpFdIn = socPair[0];
	fcntl(wakeUpFdIn, F_SETFL, (fcntl(wakeUpFdIn, F_GETFL) | O_NONBLOCK) ^ O_NONBLOCK);
	wakeUpFdOut = socPair[1];

	fcntl(socket, F_SETFL, (fcntl(socket, F_GETFL) | O_NONBLOCK) ^ O_NONBLOCK);
	pollFds[0].events = POLLIN | POLLHUP;
	pollFds[0].fd = socket;
	pollFds[1].events = POLLIN | POLLHUP;
	pollFds[1].fd = wakeUpFdIn;
}
