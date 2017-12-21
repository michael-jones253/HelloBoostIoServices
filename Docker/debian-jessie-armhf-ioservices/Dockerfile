# debian beagle crossbuild ioservices container
FROM debian-jessie-armhf-boost:latest

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update -y && apt-get install -y \
    automake \
    libtool

COPY build_context/IoServices.tar.gz /root/Crossbuilds
COPY build_context/config.h /root/Crossbuilds

WORKDIR /root/Crossbuilds
RUN tar xfz IoServices.tar.gz && mv IoServices io-services-2.0
WORKDIR /root/Crossbuilds/io-services-2.0
COPY Configure.sh /root/Crossbuilds/io-services-2.0

# because autotools does not handle dos format
# the p4v client can be set to use linux -> Connection -> Edit Current Workspace -> Advanced tab -> Line ending drop down
RUN dos2unix *.sh \
	&& dos2unix *.ac \
	&& dos2unix *.am \
	&& chmod +x ./Configure.sh \
	&& ./Configure.sh \
	&& chmod +x ./RunCrossConfigure.sh
RUN ./RunCrossConfigure.sh
RUN make \
	&& make install prefix=/opt/io_services \
	&& make dist
