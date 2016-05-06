#!/bin/bash

# Converts pem certificate made by openssl into .cer format for import into windows certificate store.

if [ $# != 2 ]; then
    echo "Usage args: <Input certificate.pem> <Output certificate.cer>"
    exit 1
fi

fd_cert_pem=$1
if [ ! -f ${fd_cert_pem} ]; then
    echo "can't find: <Input certificate>: ${fd_cert_pem}"
    exit 1
fi

if ! grep CERTIFICATE ${fd_cert_pem}; then
    echo "${fd_cert_pem} not a certificate in pem format"
    exit 1
fi

fd_cert_cer=$2

openssl x509 -inform pem -in ${fd_cert_pem} -outform der -out ${fd_cert_cer}
