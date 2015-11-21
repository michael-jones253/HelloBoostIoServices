#!/bin/bash

# https://www.openssl.org/docs/manmaster/apps/dhparam.html

# without -in will use stdin

# openssl dhparam -noout

# http://h71000.www7.hp.com/doc/83final/ba554_90007/ch06s06.html
openssl dhparam -outform PEM -out dh1024.pem 1024

# See also for example.
# http://sandilands.info/sgordon/diffie-hellman-secret-key-exchange-with-openssl
exit 0
