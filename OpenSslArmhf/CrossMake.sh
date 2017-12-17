#!/bin/bash
# make depend didn't work, just the following seemed to be needed.
# If this goes ok then sudo make install.

make CC=arm-linux-gnueabihf-gcc AR="arm-linux-gnueabihf-ar r" RANLIB="arm-linux-gnueabihf-ranlib" 2>&1 | tee make_out.txt

exit 0
