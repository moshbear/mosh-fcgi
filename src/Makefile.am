## @(#) Makefile.am - Automake file for the FastCGI++ src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

include $(top_srcdir)/include/headerlist

INCLUDES = -I$(top_srcdir)/include
AM_CPPFLAGS = $(BOOST_CPPFLAGS)
LD_HTTP = $(BOOST_REGEX_LIBS) $(BOOST_DATE_TIME_LIBS) $(CRYPTOPP_LIBS)
LD_CORE = $(BOOST_THREAD_LIBS) 
LDADD = $(LD_CORE) $(LD_HTTP)

lib_LTLIBRARIES = libmosh_fcgi.la

libmosh_fcgi_la_LDFLAGS = -version-info 0:4:0

libmosh_fcgi_la_SOURCES = $(HEADER_LIST) \
	_dummy.cpp \
	base64.cpp \
	boyer_moore.cpp \
	converter.cpp \
	exceptions.cpp \
	http_misc.cpp \
	protocol.cpp \
	qp.cpp \
	transceiver.cpp \
	unicode.cpp \
	url.cpp \
	utf8_cvt.cpp

