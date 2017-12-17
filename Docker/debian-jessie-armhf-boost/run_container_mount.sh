#!/bin/bash

# takes a host directory argument and copies the boost libraries from the container to the host directory.
# The host mount point is restricted to a directory under /c/Users in a default docker windows install

host_mount_point=$1

if [ -z "${host_mount_point}" ]; then
    echo "usage <program> /c/Users/<mount dir>"
    exit -1
fi

# interactive run of a transient container that is removed after retrieving the boost libraries archive.
docker run -it --rm --mount type=bind,source=${host_mount_point},target=/build-host-mount debian-jessie-armhf-boost:latest tar cvfz /build-host-mount/boost_1_62_0_libs.tar.gz /root/Crossbuilds/boost_1_62_0/stage/lib

exit 0
