.PHONY: all clean

all: show-gnu.fcgi

%.fcgi: %.cpp
	$(CXX11) $(CXXFLAGS) -o $@ $^ -I../../include -L../../src -lmosh_fcgi

clean:
	rm -f show-gnu.fcgi

