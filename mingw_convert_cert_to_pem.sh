#!/bin/bash

# Converts .cer certificate made by makecert into .pem format for loading into a verify file in boost

if [ $# != 2 ]; then
    echo "Usage args: <Input certificate.cer> <Output certificate.pem>"
    exit 1
fi

fd_cert_cer=$1
if [ ! -f ${fd_cert_cer} ]; then
    echo "can't find: <Input certificate>: ${fd_cert_cer}"
    exit 1
fi


fd_cert_pem=$2

openssl x509 -inform der -in ${fd_cert_cer} -outform pem -out ${fd_cert_pem}
