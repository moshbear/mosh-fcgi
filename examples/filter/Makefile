.PHONY: all clean 

all: filter.fcgi

%.fcgi: %.cpp
	$(CXX11) $(CXXFLAGS) -o $@ $^ -I../../include -L../../src/ -lmosh_fcgi

clean:
	rm -f filter.fcgi

