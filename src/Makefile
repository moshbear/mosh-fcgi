include ../Makefile.cxxopts

FCGI_CORE_FILES = $(shell find . -maxdepth 1 -name '*.cpp') $(shell find bits protocol -name '*.cpp' -o -name '*.c')

FCGI_HTTP_FILES = $(shell find http -name '*.cpp')

FCGI_HTML_FILES = $(shell find html -name '*.cpp')

all: libmosh_fcgi.so mosh_fcgi.a

libmosh_fcgi.so: core.o http.o html.o
	$(CXX11) -fPIC -shared -Wl,\( $^ -Wl,\) -lpthread -o $@

libmosh_fcgi.a: core.a http.a html.a
	$(CXX11) -Wl,\( $^ -Wl,\) -lpthread -o $@


CXX11_SRC = $(CXX11) -I../include -Iinclude

core.a core.o: core.mk
	make -f core.mk

core.mk: pre $(FCGI_CORE_FILES)
	./MakeMake "$(CXX11_SRC) $(CXXMMMQ)" "$(CXX11_SRC) $(CXXFLAGS)" core $^ > $@

http.a http.o: http.mk
	make -f http.mk

http.mk: pre $(FCGI_HTTP_FILES)
	./MakeMake "$(CXX11_SRC) $(CXXMMMQ)" "$(CXX11_SRC) $(CXXFLAGS)" http $^ > $@

html.a html.o: html.mk
	make -f html.mk

html.mk: pre $(FCGI_HTML_FILES)
	./MakeMake "$(CXX11_SRC) $(CXXMMMQ)" "$(CXX11_SRC) $(CXXFLAGS)" html $^ > $@

clean_core: core.mk
	make -f core.mk clean

clean_http: http.mk
	make -f http.mk clean

clean_html: html.mk
	make -f html.mk clean

clean: clean_core clean_http clean_html
	rm -f libmosh_fcgi.{a,so}

distclean:
	[ -e core.mk ] && make clean_core
	[ -e http.mk ] && make clean_http
	[ -e html.mk ] && make clean_html
	rm -f {core,http,html}.mk

pre: ../include/checked

.PHONY: all clean distclean clean_core clean_http clean_html pre

