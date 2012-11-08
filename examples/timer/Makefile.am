## @(#) Makefile.am - Automake file for the mosh-fcgi src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

all: timer.fcgi

timer.fcgi: timer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) -lboost_system-mt -lboost_thread-mt

clean:
	rm -f *.fcgi
