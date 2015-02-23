include ../Makefile.cxxopts

CXX11 += -I.

HEADERS = $(shell find mosh/fcgi -name '*.hpp' -o -name '*.tcc')

check: $(HEADERS)
	$(CXX11) $^
	touch checked

install:
	TODO

clean:
	find . -name '*.gch' -delete


.PHONY: check install clean
