#!/bin/bash

# If a mistake is made with Configure it is best to erase entire package
# after saving this script and then unpack it all over again.
# NB when attempting to link an application with libssl and libcrypto the order
# of the flags are critical otherwise heaps of COMP_CTX.. and pqueue_..
# undefined references will be generated. The link flags should be ordered:
# -lssl -lcrypto

OPENSSLARM=/opt/openssl_arm

CC=arm-linux-gnueabihf-gcc RANLIB=arm-linux-gnueabihf-ranlib ./Configure linux-armv4 --prefix=$OPENSSLARM --openssldir=$OPENSSLARM 2>&1 | tee configure_armv4_out.txt

# despite the output from the above command "make depend" did not work.

# I have a script to do the following.
# make CC=arm-linux-gnueabihf-gcc AR="arm-linux-gnueabihf-ar r" RANLIB="arm-linux-gnueabihf-ranlib" 2>&1 | tee make_armv4_out.txt

# This is the only step that requires sudo
sudo make install

exit 0
