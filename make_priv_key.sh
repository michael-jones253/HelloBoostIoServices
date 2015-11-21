#!/bin/bash
openssl genrsa -aes128 -out fd.key 512

openssl rsa -text -in fd-rsa-priv.key

exit 0
