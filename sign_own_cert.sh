#!/bin/bash

# creates fd.crt

if [ $# != 3 ]; then
    echo "Usage args: <Input CSR> <Input PrivateKey> <Output certificate>"
    exit 1
fi

fd_csr=$1
if [ ! -f ${fd_csr} ]; then
    echo "Usage args: <Input CSR> <Input PrivateKey> <Output certificate>"
    echo "can't find: <Input CSR>"
    exit 1
fi

if ! grep REQUEST ${fd_csr}; then
    echo "${fd_csr} not a CSR"
    exit 1
fi

fd_key=$2
if ! grep PRIVATE ${fd_key}; then
    echo "${fd_key} not a Private Key"
    exit 1
fi

fd_crt=$3
if [ -f ${fd_crt} ]; then
    echo "Not going to overwrite certificate: ${fd_crt}"
    exit 1
fi
openssl x509 -req -days 365 -in ${fd_csr} -signkey ${fd_key} -out ${fd_crt}

exit 0
