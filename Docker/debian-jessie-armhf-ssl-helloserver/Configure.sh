#!/bin/bash
# create the configure script (that's configure with a small 'c').

autoheader
aclocal
automake -c --add-missing

libtoolize -c -f
# because the first aclocal complains of not having the m4 directory re-run it
aclocal
autoconf
automake
autoreconf

exit 0
