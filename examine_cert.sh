#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage args: <Input Certificate filename>"
    exit 1
fi

fd_crt=$1

if [ ! -f ${fd_crt} ]; then
    echo "${fd_crt} not a file name"
    exit 1
fi

openssl x509 -text -in ${fd_crt} -noout

exit 0
