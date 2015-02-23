.PHONY: all clean

all: timer.fcgi

%.fcgi: %.cpp
	$(CXX11) $(CXXFLAGS) -o $@ $^ -I../../include -L../../src -lmosh_fcgi -lboost_system-mt -lboost_thread-mt

clean:
	rm -f timer.fcgi


