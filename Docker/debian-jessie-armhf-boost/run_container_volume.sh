#!/bin/bash

install_volume=$1

docker run -d -it --rm -v ${install_volume}:/build-host-mount debian-jessie-armhf-boost:latest tar cvfz //build-host-mount/boost_1_62_0_libs.tar.gz //root/Crossbuilds/boost_1_62_0/stage/lib

exit 0
