#!/bin/bash

openssl rsa -in fd.key -pubout -out fd-public.key

exit 0
