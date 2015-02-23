.PHONY: all clean

all: upload.fcgi

%.fcgi: %.cpp
	$(CXX11) $(CXXFLAGS) -o $@ $^ -I../../include -L../../src -lmosh_fcgi

clean:
	rm -f upload.fcgi

