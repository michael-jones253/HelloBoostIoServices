#!/bin/bash

# create a temporary directory for build context files that are not checked into perforce
mkdir -p build_context

# copy from docker forbidden path into build context
# these files will get new timestamp which will force a build cache invalidate resulting in a clean build
cp -f ../../BoostArmhf/boost_1_62_0.tar.bz2 build_context
cp -f ../../BoostArmhf/user-config.jam build_context
cp -f ../../OpenSslArmhf/openssl-1.0.2j.tar.gz build_context
cp -f ../../OpenSslArmhf/CrossCompileOpenSsl.sh build_context
cp -f ../../OpenSslArmhf/CrossMake.sh build_context

# build the docker image with a build context of this directory
docker build -t debian-jessie-armhf-boost .

# remove temporary build context
rm -rf build_context

exit 0
