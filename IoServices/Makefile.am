# Build a library called libioservices
lib_LTLIBRARIES = libioservices.la

libioservices_la_SOURCES = DgramListener.cpp IoBufferWrapper.cpp IoCircularBuffer.cpp IoLogConsumer.cpp IoLog.cpp IoServices.cpp IoServicesImpl.cpp StreamConnection.cpp TcpConnection.cpp TcpPeerConnection.cpp TcpServer.cpp TcpSslConnection.cpp UdpListener.cpp IoEndPoint.cpp UdpBufferWrapper.cpp TcpDomainConnection.cpp UnixStreamConnection.cpp UnixServer.cpp

# Public library headers. Private ones can go in the SOURCES line above.
include_HEADERS = DgramListener.h IoBufferWrapper.h IoCircularBuffer.h IoLogConsumer.h IoLog.h IoServices.h IoServicesImpl.h IoServicesPriv.h PeriodicTimer.h SecurityOptions.h stdafx.h StreamConnection.h targetver.h TcpPeerConnection.h TcpServer.h Timer.h UdpListener.h TcpSslConnection.hpp TcpConnection.hpp IoEndPoint.h UdpBufferWrapper.h TcpDomainConnection.h UnixStreamConnection.h UnixServer.h

#AM_CPPFLAGS = -I/opt/openssl_arm/include -I/home/mikej/boost_1_61_0
AM_CPPFLAGS = -I/opt/openssl_1_0_2/include -I/opt/boost_1_62_0/include
AM_CXXFLAGS = -std=gnu++14
libioservices_la_LDFLAGS = -static

foo-local:
	@echo This is ${srcdir} from srclib
