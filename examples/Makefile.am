## @(#) Makefile.am - Automake file for the FastCGI++ src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

EXTRA_DIST = echo-form.html gnu.png upload.html echo.cpp showgnu.cpp upload.cpp utf8-helloworld.cpp timer.cpp koi8r.cpp filter.cpp

all: examples

examples: utf8-helloworld.fcgi echo.fcgi showgnu.fcgi timer.fcgi upload.fcgi koi8r.fcgi filter.fcgi

%.fcgi: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) $(CXXFLAGS) -lnss3 $(NSS_CFLAGS)

timer.fcgi: timer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) $(CXXFLAGS) -lnss3 $(NSS_CFLAGS) -lboost_system-mt -lboost_thread-mt

clean:
	rm -f *.fcgi
