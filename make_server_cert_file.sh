#!/bin/bash

cat fd.crt > fd-serv.crt
cat fd-public.key >> fd-serv.crt

exit 0
