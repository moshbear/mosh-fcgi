## @(#) Makefile.am - Automake file for the mosh-fcgi src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

all: raw-echo.fcgi

%.fcgi: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) 

clean:
	rm -f *.fcgi
