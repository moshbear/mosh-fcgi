ifndef PREFIX
	PREFIX = /usr/local
endif

ifndef WWWROOT
	WWWROOT = /var/www/localhost/htdocs
endif

ifdef STATIC
	STATIC = -static
endif

ifdef STRIP
	STRIP = true
else
	STRIP = false
endif

ifndef CXX
	CXX = g++
endif
ifndef CXXFLAGS
	CXXFLAGS = -Wall -Wextra
endif

ifndef BOOST_LDIR
	BOOST_LDIR = /usr/lib
endif

DOSTRIP=strip --strip-unneeded --strip-debug

BOOST_LIBS = $(addsuffix -mt, $(addprefix -lboost_, regex))

all: lib include

install: lib include
	mkdir -p $(PREFIX)/include/fastcgipp-mosh
	install -o 0 -g 0 -m 644 include/fastcgipp-mosh/*.hpp $(PREFIX)/include/fastcgipp-mosh
	install -o 0 -g 0 -m 644 lib/libfastcgipp.a $(PREFIX)/lib/libfastcgipp.a
	install -o 0 -g 0 -m 755 lib/libfastcgipp.so $(PREFIX)/lib/libfastcgipp.so

clean: libclean 

uninstall:
	rm -f $(PREFIX)/lib/libfastcgipp.a $(PREFIX)/lib/libfastcgipp.so
	rm -f $(PREFIX)/include/fastcgipp-mosh/*.hpp
	rm -f $(PREFIX)/share/doc/fastcgipp-mosh/*
	rm -f $(WWWROOT)/fastcgipp/*
	rmdir $(WWWROOT)/fastcgipp; true
	irmdir $(PREFIX)/include/fastcgipp-mosh; true
	rmdir $(PREFIX)/share/doc/fastcgipp-mosh; true

doc: include doxygen lib/src/*.cpp doc.hpp
	VERSION=1.2 DATE="Dec 11, 2008" doxygen doxygen

doc-install:
	mkdir $(PREFIX)/share/doc/fastcgipp-mosh
	install -o 0 -g 0 -m 644 doc/* $(PREFIX)/share/doc/fastcgipp-mosh

docclean:
	rm -rf doc

INCLUDE = -Iinclude
IDIR = include/fastcgipp-mosh
IDIR_BITS = include/fastcgipp-mosh/bits
IDIR_BITS_HTTP = include/fastcgipp-mosh/bits/http

include: $(addprefix $(IDIR)/, exceptions.hpp fcgistream.hpp http/http.hpp http/http.tcc manager.hpp protocol.hpp request.hpp transceiver.hpp unicode.hpp) \
$(addprefix $(IDIR_BITS)/, append_or_assign.hpp boyer_moore.hpp cmp.hpp singleton.hpp stdint.hpp vec_ops.hpp utf8_cvt.hpp) \
$(addprefix $(IDIR_BITS_HTTP), form.hpp url.hpp)
	

$(IDIR)/fcgistream.hpp: $(IDIR)/protocol.hpp
	

$(IDIR)/http.hpp: $(addprefix $(IDIR)/, exceptions.hpp protocol.hpp) $(addprefix $(IDIR)/http/, http.hpp http.tcc)
	

$(IDIR)/manager.hpp: $(addprefix $(IDIR)/, exceptions.hpp protocol.hpp transceiver.hpp)
	

$(IDIR)/request.hpp: $(addprefix $(IDIR)/, protocol.hpp exceptions.hpp transceiver.hpp fcgistream.hpp http.hpp)
	

$(IDIR)/transceiver.hpp: $(addprefix $(IDIR)/, protocol.hpp exceptions.hpp)
	

$(IDIR)/unicode.hpp:
	

lib: lib/libfastcgipp.a lib/libfastcgipp.so

lib_SRCS = protocol fcgistream transceiver exceptions unicode boyer_moore utf8_cvt
libclean:
	rm -f lib/*.so lib/*.a lib/*.o


lib/libfastcgipp.a: $(addsuffix -static.o, $(addprefix lib/, $(lib_SRCS)))
	ar rc $@ $^
	./ar-merge $@ $(addsuffix -mt.a, $(addprefix $(BOOST_LDIR)/libboost_, regex))
	ranlib $@
ifeq ($(STRIP),true)
	 strip --strip-unneeded --strip-debug $@
endif

lib/libfastcgipp.so: $(addsuffix -shared.o, $(addprefix lib/, $(lib_SRCS)))
	$(CXX) $(CXXFLAGS) -shared $(BOOST_LIBS) -o $@ $^ $(INCLUDE) 
ifeq ($(STRIP),true)
	strip -s $@
endif

lib/boyer_moore-static.o: src/boyer_moore.cpp include/fastcgipp-mosh/bits/boyer_moore.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INCLUDE)
	
lib/boyer_moore-shared.o: src/boyer_moore.cpp include/fastcgipp-mosh/bits/boyer_moore.hpp
	$(CXX) $(CXXFLAGS) -fPIC -o $@ -c $< $(INCLUDE)
	
lib/utf8_cvt-static.o: src/utf8_cvt.cpp include/fastcgipp-mosh/bits/utf8_cvt.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INCLUDE)
	
lib/utf8_cvt-shared.o: src/utf8_cvt.cpp include/fastcgipp-mosh/bits/utf8_cvt.hpp
	$(CXX) $(CXXFLAGS) -fPIC -o $@ -c $< $(INCLUDE)
	

lib/%-static.o: src/%.cpp include/fastcgipp-mosh/%.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INCLUDE)

lib/%-shared.o: src/%.cpp include/fastcgi-mosh/%.hpp
	$(CXX) $(CXXFLAGS) -fPIC -o $@ -c $< $(INCLUDE)
