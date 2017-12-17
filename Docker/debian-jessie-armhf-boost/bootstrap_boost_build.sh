#!/bin/bash

# First install boost build (this is not cross compiled)

boost_prefix=boost_1_62_0

if [ ! -d "${boost_prefix}" ]; then
    tar xjf "${boost_prefix}.tar.bz2"
fi

cd "${boost_prefix}"
ls tools/
  
# NB this is important cd tools/build directory to bootstrap.
cd tools/build

# bootstrap to get a b2 exe here.
./bootstrap.sh 2>&1 | tee bootstrap_out.txt
mkdir --parents /opt/boost_build

# run the bootstrapped b2 to build and install an operating b2 (boost build tool)
./b2 install --prefix=/opt/boost_build 2>&1 | tee b2_install_out.txt

exit 0
