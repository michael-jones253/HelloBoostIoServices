#!/bin/bash

# creates fd.crt

openssl x509 -req -days 365 -in fd.csr -signkey fd.key -out fd.crt

exit 0
