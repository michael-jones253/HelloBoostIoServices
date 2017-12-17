#!/bin/bash

# create a temporary directory for build context files that are not checked into perforce
mkdir -p build_context

# creating a new archive every time will invalidate the docker build cache resulting in a clean build.
tar cvfz build_context/io-services-2.0.tar.gz ../../io-services-2.0/

# build the docker image with a build context of this directory
docker build -t debian-jessie-armhf-ioservices .

# remove temporary build context
rm -rf build_context

exit 0
