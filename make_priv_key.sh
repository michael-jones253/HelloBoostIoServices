#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage args: <Output Key filename>"
    exit 1
fi

fd_key=$1

openssl genrsa -aes128 -out ${fd_key} 512

openssl rsa -text -in ${fd_key}

exit 0
