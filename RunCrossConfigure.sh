#!/bin/bash

./configure --host=arm-linux-gnueabihf --prefix=/opt/io_services 2>&1 | tee Configure.out

# cd IoServices
# sudo make install prefix=/opt/io_services

exit 0
