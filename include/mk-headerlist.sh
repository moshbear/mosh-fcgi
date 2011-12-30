#!/bin/sh
echo -n 'Generating include/headerlist ... '
echo '#Automatically generated list of header files by mk-headerlist.sh, v1' > headerlist
echo 'HEADER_LIST = \' >> headerlist
find mosh/fcgi -type f -name '*.hpp' -o -name '*.tcc' | sed '$!s/$/ \\/' >> headerlist
echo >> headerlist
echo -n '('$(find mosh/fcgi -type f -name '*.hpp' -o -name '*.tcc' | wc -l)')'
echo ' done'

