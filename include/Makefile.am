## @(#) Makefile.am - Automake file for the mosh-fcgi interface files
##
## $Id$
##

DISTCLEANFILES = Makefile Makefile.in

include $(top_srcdir)/include/headerlist

nobase_include_HEADERS =  $(HEADER_LIST)

# Have CXX process the headers before examples, so that syntactical and semantical errors can be caught
# at an earlier stage in the build.

mosh/fcgi/bits/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/fcgistream/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/fcgistream/%.tcc:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/html/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/html/element/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/http/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/http/conv/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/http/helpers/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/http/session/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@

mosh/fcgi/manager/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@
	
mosh/fcgi/protocol/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@
	
mosh/fcgi/%.hpp:
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(CFLAGS) $@ || echo $(top_srcdir)/include/$@
	

