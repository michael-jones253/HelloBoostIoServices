#!/bin/bash

./configure --host=arm-linux-gnueabihf --prefix=/opt/local 2>&1 | tee Configure.out

# sudo make install prefix=/opt/local

exit 0
