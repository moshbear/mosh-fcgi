## @(#) Makefile.am - Automake file for the mosh-fcgi src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

EXTRA_DIST = echo.html gnu.png upload.html echo.cpp raw-echo.cpp show-gnu.cpp upload.cpp utf8-helloworld.cpp timer.cpp \
		filter.cpp

all: examples

examples: utf8-helloworld.fcgi echo.fcgi show-gnu.fcgi timer.fcgi upload.fcgi filter.fcgi

%.fcgi: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) 

timer.fcgi: timer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) -lboost_system-mt -lboost_thread-mt

clean:
	rm -f *.fcgi
