#!/bin/bash

# creates fd.csr

openssl req -new -key fd.key -out fd.csr

# check csr
openssl req -text -in fd.csr -noout

exit 0
