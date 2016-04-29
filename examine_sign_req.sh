#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage args: <Input Certificate sign request filename>"
    exit 1
fi

fd_csr=$1

if [ ! -f ${fd_csr} ]; then
    echo "${fd_csr} not a file name"
    exit 1
fi

openssl req -text -in ${fd_csr} -noout

exit 0
