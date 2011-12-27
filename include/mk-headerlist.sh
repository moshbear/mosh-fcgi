#!/bin/sh
echo -n 'Generating include/headerlist...'
echo '#Automatically generated list of header files by mk-headerlist.sh, v0' > headerlist
echo 'HEADER_LIST = \' >> headerlist
find mosh/fcgi -type f -name '*.hpp' -o -name '*.tcc' | sed '$!s/$/ \\/' >> headerlist
echo >> headerlist
echo 'done'

