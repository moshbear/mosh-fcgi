## @(#) Makefile.am - Automake file for the mosh-fcgi interface files
##
## $Id$
##

DISTCLEANFILES = Makefile Makefile.in

include $(top_srcdir)/include/headerlist

nobase_include_HEADERS =  $(HEADER_LIST)

HEADERS: $(HEADER_LIST)

INCLUDES = -I$(top_srcdir)/include

$(HEADER_LIST): hdone
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $(NSS_CFLAGS) $@ || echo $(top_srcdir)/include/$@
	touch hdone

mosh/fcgi/cgi_request.hpp: hdone
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $(NSS_CFLAGS) -DMOSH_FCGI_USE_CGI=1 $@ || echo $(top_srcdir)/include/$@ 
	touch hdone


hdone:


