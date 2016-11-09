AM_CPPFLAGS = -I../IoServices -I/home/mikej/boost_1_61_0
AM_CXXFLAGS = -std=gnu++14
# could also use unix_domain_client_LDFLAGS if want to override AM_LDFLAGS.
# Do not strip -s is the strip symbols flag
#AM_LDFLAGS = -lpthread -lrt -s
#AM_LDFLAGS = -pthread -lrt -static
AM_LDFLAGS = -lioservices -lboost_system -lboost_thread -lboost_program_options -lssl -lcrypto -pthread -lrt -ldl -static -L../IoServices/.libs -L/home/mikej/boost_1_61_0/stage/lib -L/opt/openssl_arm/lib -rpath /home/debian

# Make a program called unix_domain_client
bin_PROGRAMS = unix_domain_client

unix_domain_client_SOURCES = main.cpp ClientUnixConnection.cpp ClientUnixConnection.h UnixConnectionManager.cpp UnixConnectionManager.h
# We only want to link ioservices static so we specify the flag in IoServices
#unix_domain_client_LDFLAGS = -static

foo-local:
	@echo This is ${srcdir} from src

