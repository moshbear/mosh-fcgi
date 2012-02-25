#!/bin/sh
echo '**mk-headerslist**'
cd include
./mk-headerlist.sh
cd ..
echo '**autogen**'
./autogen.sh
echo '**configure**'
./configure CXX="ccache g++ -std=c++0x"
echo '**make**'
make
