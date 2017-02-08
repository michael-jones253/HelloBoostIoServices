AM_CPPFLAGS = -I../IoServices -I/opt/boost_1_62_0/include
AM_CXXFLAGS = -std=gnu++14
# could also use unix_domain_server_LDFLAGS if want to override AM_LDFLAGS.
# Do not strip -s is the strip symbols flag
#AM_LDFLAGS = -lpthread -lrt -s
#AM_LDFLAGS = -pthread -lrt -static
AM_LDFLAGS = -lioservices -lboost_system -lboost_thread -lssl -lcrypto -pthread -lrt -ldl -static -L../IoServices/.libs -L/opt/boost_1_62_0/lib -L/opt/openssl_1_0_2/lib -rpath /home/debian

# Make a program called unix_domain_server
bin_PROGRAMS = unix_domain_server

unix_domain_server_SOURCES = main.cpp
# We only want to link ioservices static so we specify the flag in IoServices
#unix_domain_server_LDFLAGS = -static

foo-local:
	@echo This is ${srcdir} from src

