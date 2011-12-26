## @(#) Makefile.am - Automake file for the FastCGI++ src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

include $(top_srcdir)/include/headerlist

INCLUDES = -I$(top_srcdir)/include
AM_CPPFLAGS = $(BOOST_CPPFLAGS)
LD_HTTP = -lnss3
LD_CORE = $(BOOST_THREAD_LIBS) 
LDADD = $(LD_CORE) $(LD_HTTP)

FCGI_FILES = 	boyer_moore.cpp \
		codecvt_ic.cpp \
		exceptions.cpp \
		protocol.cpp \
		transceiver.cpp

FCGI_HTTP_FILES = 	http/misc.cpp \
			http/converter.cpp \
			http/base64.cpp \
			http/qp.cpp \
			http/url.cpp

FCGI_HTTP_FILES_2 = 	http/cookie.cpp \
			http/header_helper/content_type.cpp \
			http/header_helper/redirect.cpp \
			http/header_helper/response.cpp \
			http/header_helper/status.cpp \
			http/header_helper/status_helper.cpp

FCGI_HTML_FILES =	html/html_doctype.cpp

lib_LTLIBRARIES = libmosh_fcgi.la

libmosh_fcgi_la_LDFLAGS = -version-info 0:6:0

libmosh_fcgi_la_SOURCES = $(HEADER_LIST) $(FCGI_FILES) $(FCGI_HTTP_FILES) $(FCGI_HTTP_FILES_2) $(FCGI_HTML_FILES)
				
