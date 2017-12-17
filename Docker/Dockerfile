# debian beagle crossbuild container
From debian:jessie
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update -y && apt-get install -y \
    apt-utils \
    build-essential \
    curl \
    vim \
    lsb-release

# see //MediaPortal/main/ArmhfCrossbuild/Debian/README_debian_cross_compile_arm.txt

COPY crosstools.list /etc/apt/sources.list.d

RUN curl http://emdebian.org/tools/debian/emdebian-toolchain-archive.key | apt-key add -
RUN dpkg --add-architecture armhf
RUN apt-get update -y && apt-get install -y \
    crossbuild-essential-armhf -y

# see //MediaPortal/main/ArmhfCrossbuild/BoostArmhf/MJ_BUILD_CROSS_BOOST.txt
RUN mkdir /root/Crossbuilds

# now set path in .bashrc to use operating b2 which will be built in another layered container
RUN echo "PATH=/opt/boost_build/bin:$PATH" >> /root/.bashrc
RUN echo "PATH=/opt/boost_build/bin:$PATH" >> /root/.bash_profile

CMD cat /etc/apt/sources.list.d/crosstools.list
CMD echo
CMD cd /root/Crossbuilds; ls

