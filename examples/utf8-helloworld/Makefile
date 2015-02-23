.PHONY: all clean

all: utf8-helloworld.fcgi

%.fcgi: %.cpp
	$(CXX11) $(CXXFLAGS) -o $@ $^ -I../../include -L../../src -lmosh_fcgi

clean:
	rm -f utf8-helloworld.fcgi

