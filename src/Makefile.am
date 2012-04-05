## @(#) Makefile.am - Automake file for the mosh-fcgi src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

include $(top_srcdir)/include/headerlist

INCLUDES = -I$(top_srcdir)/include 
AM_CPPFLAGS = $(BOOST_CPPFLAGS) -Iinclude

# mosh-fcgi core
FCGI_FILES = 	bits/boyer_moore.cpp \
		bits/hash.cpp \
		bits/rwlock.cpp \
		bits/sha/sha1.c \
		bits/tempfile.cpp \
		bits/utf8.cpp \
		exceptions.cpp \
		fcgistream.cpp \
		manager.cpp \
		protocol/funcs.cpp \
		protocol/message.cpp \
		protocol/vars.cpp \
		request.cpp \
		transceiver.cpp

# mosh-fcgi http handling
FCGI_HTTP_FILES = 	http/misc.cpp \
			http/converter.cpp \
			http/base64.cpp \
			http/qp.cpp \
			http/session.cpp \
			http/session-timpl.cpp \
			http/url.cpp

# mosh-fcgi http handling imported from mosh-cgi
FCGI_HTTP_FILES_2 = 	http/header/header.cpp \
			http/header/helper.cpp \
			http/header/content_type.cpp \
			http/header/redirect.cpp \
			http/header/response.cpp \
			http/header/status.cpp \
			http/header/x_sendfile.cpp \
			http/cookie.cpp

# mosh-fcgi html handling imported from mosh-cgi
FCGI_HTML_FILES =	html/html_doctype.cpp

lib_LTLIBRARIES = libmosh_fcgi.la

libmosh_fcgi_la_LDFLAGS = -version-info 0:7:0 -lpthread  $(BOOST_THREAD_LIBS)

libmosh_fcgi_la_SOURCES = $(HEADER_LIST) $(FCGI_FILES) $(FCGI_HTTP_FILES) $(FCGI_HTTP_FILES_2) $(FCGI_HTML_FILES)


