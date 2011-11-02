#!/bin/sh
echo '#Automatically generated list of header files by mk-headerlist.sh, v0' > headerlist
echo 'HEADER_LIST = \' >> headerlist
find mosh/fcgi -type f | sed '$!s/$/ \\/' >> headerlist
echo >> headerlist

