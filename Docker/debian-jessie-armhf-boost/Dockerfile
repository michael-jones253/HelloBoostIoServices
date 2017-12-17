# debian beagle crossbuild boost container
FROM debian-jessie-crossbuild:latest

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update -y && apt-get install -y \
    dos2unix

# see //MediaPortal/main/ArmhfCrossbuild/BoostArmhf/MJ_BUILD_CROSS_BOOST.txt
COPY build_context/boost_1_62_0.tar.bz2 /root/Crossbuilds
COPY build_context/user-config.jam /root/Crossbuilds
COPY bootstrap_boost_build.sh /root/Crossbuilds
COPY build_boost_armhf.sh /root/Crossbuilds
COPY install_boost.sh /root/Crossbuilds
RUN dos2unix /root/Crossbuilds/*.sh
RUN chmod +x /root/Crossbuilds/*.sh

WORKDIR /root/Crossbuilds
RUN ./bootstrap_boost_build.sh

ENV PATH=/opt/boost_build/bin:$PATH
# the build takes a long time
RUN ./build_boost_armhf.sh
RUN ./install_boost.sh

COPY build_context/openssl-1.0.2j.tar.gz /root/Crossbuilds

# build openssl
WORKDIR /root/Crossbuilds
RUN tar xfz openssl-1.0.2j.tar.gz
COPY build_context/CrossCompileOpenSsl.sh /root/Crossbuilds/openssl-1.0.2j
COPY build_context/CrossMake.sh /root/Crossbuilds/openssl-1.0.2j
RUN dos2unix /root/Crossbuilds/openssl-1.0.2j/*.sh
RUN chmod +x  /root/Crossbuilds/openssl-1.0.2j/*.sh
WORKDIR /root/Crossbuilds/openssl-1.0.2j
RUN ./CrossCompileOpenSsl.sh
RUN ./CrossMake.sh
RUN make install

