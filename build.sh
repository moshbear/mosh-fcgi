#!/bin/sh
cd include
./mk-headerlist.sh
cd ..
./autogen.sh
./configure CXX="ccache g++ -std=c++0x"
make
