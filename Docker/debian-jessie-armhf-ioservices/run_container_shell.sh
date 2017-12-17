#!/bin/bash

# development helper script to run a temporary container with an interactive bash shell so we can take a look around
docker run --rm -it debian-jessie-armhf-ioservices /bin/bash

exit 0
