# debian beagle crossbuild ioservices container
FROM debian-jessie-armhf-ioservices:latest

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update -y && apt-get install -y \
    automake \
    libtool

COPY build_context/RunSslServer.tar.gz /root/Crossbuilds
COPY build_context/config.h /root/Crossbuilds

WORKDIR /root/Crossbuilds
RUN tar xfz RunSslServer.tar.gz && mv RunSslServer ssl-helloserver
WORKDIR /root/Crossbuilds/ssl-helloserver
COPY Configure.sh /root/Crossbuilds/ssl-helloserver

# because autotools does not handle dos format
# the p4v client can be set to use linux -> Connection -> Edit Current Workspace -> Advanced tab -> Line ending drop down
RUN dos2unix *.sh \
	&& dos2unix *.ac \
	&& dos2unix *.am \
	&& chmod +x ./Configure.sh \
	&& ./Configure.sh \
	&& chmod +x ./RunCrossConfigure.sh
RUN ./RunCrossConfigure.sh
#RUN make \
#	&& make dist
