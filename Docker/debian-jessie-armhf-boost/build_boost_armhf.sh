#!/bin/bash

boost_prefix=boost_1_62_0

which b2
if [ $? -ne 0 ]; then
    echo "operating b2 not found"
    exit -1
fi

# change directory to top level unpacked boost directory
cd "/root/Crossbuilds/${boost_prefix}"

# the jam file should already be setup for a crossbuild which means:
# comment out the gcc line from example user-config.jam and replace as follows:
# using gcc : 3.2 : g++-3.2 ;
# and replace with:
# using gcc : arm : arm-linux-gnueabihf-g++ ;

# copy jam file into home directory
# This is important otherwise it won't be picked up. See boost build docs for
# search order of jam files.
cp /root/Crossbuilds/user-config.jam ~

# Now cross compile into the stage directory.
b2 toolset=gcc-arm stage 2>&1 | tee b2_toolset_gcc_arm_out.txt

exit 0
