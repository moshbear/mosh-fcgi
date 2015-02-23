.PHONY: all clean

all: raw-echo.fcgi

%.fcgi: %.cpp
	$(CXX11) $(CXXFLAGS) -o $@ $^ -I../../include -L../../src -lmosh_fcgi

clean:
	rm -f raw-echo.fcgi

