#!/bin/bash

boost_prefix=boost_1_62_0

mkdir --parents "/opt/${boost_prefix}/include"
ln -s "/root/Crossbuilds/${boost_prefix}/stage/lib" "/opt/${boost_prefix}"
ln -s "/root/Crossbuilds/${boost_prefix}/boost" "/opt/${boost_prefix}/include"

exit 0
