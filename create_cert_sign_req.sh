#!/bin/bash

# creates certificate sign request (CSR).

if [ $# -ne 2 ]; then
    echo "Usage args: <Input Private Key filename> <Output cert req>"
    exit 1
fi

fd_key=$1

if [ ! -f ${fd_key} ]; then
    echo "Usage args: <Input Private Key filename> <Output cert req>"
    echo "Input Private Key file: ${fd_key} not found."
    exit 1
fi

if ! grep PRIVATE ${fd_key}; then
    echo "Usage args: <Input Private Key filename> <Output cert req>"
    echo "Input Private Key file: ${fd_key} not a private key."
    exit 1
fi

fd_csr=$2

if [ -f ${fd_csr} ]; then
    echo "NOT going to overwrite existing ${fd_csr}"
    exit 1
fi

openssl req -new -key ${fd_key} -out ${fd_csr}

# check CSR
openssl req -text -in ${fd_csr} -noout

exit 0
