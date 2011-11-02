## @(#) Makefile.am - Automake file for the FastCGI++ src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

EXTRA_DIST = echo-form.html gnu.png upload.html echo.cpp showgnu.cpp upload.cpp utf8-helloworld.cpp timer.cpp

all: examples

examples: utf8-helloworld.fcgi echo.fcgi showgnu.fcgi timer.fcgi upload.fcgi

utf8-helloworld.fcgi: utf8-helloworld.cpp
	$(CXX) -o utf8-helloworld.fcgi utf8-helloworld.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) $(CXXFLAGS) -lcrypto++

echo.fcgi: echo.cpp
	$(CXX) -o echo.fcgi echo.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) $(CXXFLAGS) -lcrypto++

showgnu.fcgi: showgnu.cpp
	$(CXX) -o showgnu.fcgi showgnu.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) $(CXXFLAGS) -lcrypto++

timer.fcgi: timer.cpp
	$(CXX) -o timer.fcgi timer.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) $(BOOST_SYSTEM_LIBS) $(CXXFLAGS) -lcrypto++

upload.fcgi: upload.cpp
	$(CXX) -o upload.fcgi upload.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src/.libs $(pkgConfigLibs) $(CXXFLAGS) -lcrypto++

clean:
	rm -f *.fcgi
