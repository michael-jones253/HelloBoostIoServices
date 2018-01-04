AM_CPPFLAGS = -I/opt/io_services/include -I/opt/boost_1_62_0/include -I/opt/openssl_arm/include
AM_CXXFLAGS = -std=gnu++14
# could also use ssl_hello_server_LDFLAGS if want to override AM_LDFLAGS.
# Do not strip -s is the strip symbols flag
#AM_LDFLAGS = -lpthread -lrt -s
#AM_LDFLAGS = -pthread -lrt -static
#ssl_hello_server_LDADD = /opt/io_services/lib/libioservices.la

AM_LDFLAGS = -lioservices -L/opt/io_services/lib -lboost_system -lboost_thread -lboost_program_options -lssl -lcrypto -pthread -lrt -ldl -L/opt/boost_1_62_0/lib -L/opt/openssl_arm/lib -rpath /home/debian

# Make a program called ssl_hello_server
bin_PROGRAMS = ssl_hello_server

ssl_hello_server_SOURCES = main.cpp

EXTRA_DIST = RunCrossConfigure.sh

foo-local:
	@echo This is ${srcdir} from src

