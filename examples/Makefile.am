## @(#) Makefile.am - Automake file for the FastCGI++ src directory
##
## $Id$
##

DISTCLEANFILES = Makefile.in Makefile

EXTRA_DIST = echo-form.html gnu.png upload.html echo.cpp showgnu.cpp upload.cpp utf8-helloworld.cpp timer.cpp authorizer.cpp locked/locked.png

examples: utf8-helloworld.fcgi echo.fcgi showgnu.fcgi timer.fcgi upload.fcgi authorizer.fcgi

utf8-helloworld.fcgi: utf8-helloworld.cpp
	$(CXX) -o utf8-helloworld.fcgi utf8-helloworld.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src $(pkgConfigLibs) $(CXXFLAGS)

echo.fcgi: echo.cpp
	$(CXX) -o echo.fcgi echo.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src $(pkgConfigLibs) $(CXXFLAGS)

showgnu.fcgi: showgnu.cpp
	$(CXX) -o showgnu.fcgi showgnu.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src $(pkgConfigLibs) $(CXXFLAGS)

timer.fcgi: timer.cpp
	$(CXX) -o timer.fcgi timer.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src $(pkgConfigLibs) $(BOOST_SYSTEM_LIBS) $(CXXFLAGS)

upload.fcgi: upload.cpp
	$(CXX) -o upload.fcgi upload.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src $(pkgConfigLibs) $(CXXFLAGS)

authorizer.fcgi: authorizer.cpp
	$(CXX) -o authorizer.fcgi authorizer.cpp -I$(top_srcdir)/include -L$(top_srcdir)/src $(pkgConfigLibs) $(CXXFLAGS)

clean:
	rm -f *.fcgi
