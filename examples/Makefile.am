## @(#) Makefile.am - Automake file for the mosh-fcgi src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

EXTRA_DIST = echo-form.html gnu.png upload.html echo.cpp showgnu.cpp upload.cpp utf8-helloworld.cpp timer.cpp \
		koi8r.cpp filter.cpp utf8-helloworld-cgi.cpp

all: examples

examples: utf8-helloworld.fcgi echo.fcgi showgnu.fcgi timer.fcgi upload.fcgi koi8r.fcgi filter.fcgi utf8-helloworld.cgi

%.fcgi: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) -lnss3 $(NSS_CFLAGS)

timer.fcgi: timer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) -lnss3 $(NSS_CFLAGS) -lboost_system-mt -lboost_thread-mt

# We still need most of the lib for cgi too, so the make rule, unsurprisingly, is effectively the same, with the
# exception of -DMOSH_FCGI_USE_CGI
%.cgi: %-cgi.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) -lnss3 $(NSS_CFLAGS) -DMOSH_FCGI_USE_CGI

clean:
	rm -f *.fcgi
