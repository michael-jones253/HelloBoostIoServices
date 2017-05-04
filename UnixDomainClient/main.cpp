// UDP to USB server

#include "UnixConnectionManager.h"

#include <IoServices/IoLogConsumer.h>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <unistd.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <sstream>
#include <atomic>
#include <future>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <stdexcept>
#include <errno.h>
#include <syslog.h>

using namespace UnixClient;
using namespace AsyncIo;
using namespace std;
using namespace std::chrono;

namespace argopt = boost::program_options;
namespace etc = boost::property_tree;

class SigHandler {
private:
    atomic<bool> _shouldRun{};
    int _watchdogHandle{};
    bool _magicClose{};
    future<void> _watchdog;
    IoServices& _services;
    UnixConnectionManager& _mgr;
    int _watchdogTimeout;

public:
    SigHandler(IoServices& services, UnixConnectionManager& mgr, int timeout, bool magicClose) :
        _magicClose{ magicClose },
        _services{ services },
        _mgr{ mgr },
        _watchdogTimeout{ timeout }
    {
        _watchdogHandle = open("/dev/watchdog", O_RDWR);
        int interval{ _watchdogTimeout };
        ioctl(_watchdogHandle, WDIOC_SETTIMEOUT, &interval);

        watchdog_info info{};
        ioctl(_watchdogHandle, WDIOC_GETSUPPORT, &info);
        if (info.options & WDIOF_MAGICCLOSE) {
            syslog(LOG_INFO, "magic close supported");
        }

        _shouldRun.store(true);
        auto task = [this]() {
            while(_shouldRun.load()) {
                ioctl(_watchdogHandle, WDIOC_KEEPALIVE, 0);
                this_thread::sleep_for(seconds(1));
            }
        };

        _watchdog = async(launch::async, (move(task)));
    }

    ~SigHandler() {
        syslog(LOG_INFO, "handling signal");
        _shouldRun.store(false);
        _mgr.Stop();
        _services.Stop();

        if (_magicClose) {
            vector<char> magic{ 'V' };
            auto count = write(_watchdogHandle, magic.data(), 1);
            syslog(LOG_INFO, "magic close wrote %d", count);
        }

        close(_watchdogHandle);

        _watchdog.get();
    }
};

shared_ptr<SigHandler> sigHandlerHandle;

void sig_handler(int signum)
{
#if __GNUC__>4
    // Ubuntu xenial has atomic_load
	auto handle = atomic_load(&sigHandlerHandle);
    if (handle) {
        sigHandlerHandle.reset();
    }
#else
    // Debian jessie is on GCC 4.9
    if (sigHandlerHandle) {
        sigHandlerHandle.reset();
    }
#endif
}


int main(int argc, char *argv[])
{
	setlogmask(LOG_UPTO(LOG_DEBUG));
	// set up signal action
	struct sigaction action{};
	action.sa_handler = sig_handler;
	sigaction(SIGHUP, &action, 0);
	sigaction(SIGTERM, &action, 0);

    try
	{
        argopt::options_description theOptions("Allowed args");
        theOptions.add_options()
        ("path", argopt::value<std::string>()->required(), "domain socket path")
        ("connect-timeout", argopt::value<int>()->default_value(5), "connect timeout")
        ("watchdog-timeout", argopt::value<int>()->default_value(5), "watchdog timeout")
        ("startup-sleep", argopt::value<int>()->default_value(0), "sleep at startup for gdb attach")
        ("magic-close", "watchdog magic close");

        argopt::variables_map args;
        auto parsed = argopt::command_line_parser(argc, argv).options(theOptions).run();

        argopt:store(parsed, args);

        string domainPath("/var/run/domain");
        seconds connectTimeout;
        seconds heartbeatTimeout;
        seconds startupSleep;
        int watchdogTimeout;

        if (args.count("path") > 0)
        {
            domainPath = args["path"].as<std::string>();
        }

        if (args.count("connect-timeout") > 0)
        {
            auto timeout  = args["connect-timeout"].as<int>();
            connectTimeout = seconds(timeout);
        }

        if (args.count("watchdog-timeout") > 0)
        {
            auto timeout  = args["watchdog-timeout"].as<int>();
            watchdogTimeout = timeout;
        }

        if (args.count("startup-sleep") > 0)
        {
            auto sleepTime  = args["startup-sleep"].as<int>();
            startupSleep = seconds{ sleepTime };
        }

        bool magicClose{};
        if (args.count("magic-close") > 0)
        {
            magicClose = true;
        }

        openlog("unix-optimeye-client", LOG_NDELAY | LOG_PID, LOG_LOCAL1);

        this_thread::sleep_for(startupSleep);

        auto path = "/etc/catapult/optimeye.ini";
        ifstream configFile(path);
        if (!configFile.good())
        {
            throw runtime_error(string("Master service failed to load: ") + path);
        }

        try {
            etc::ptree pt;
            syslog(LOG_INFO, "reading ini");
            etc::ini_parser::read_ini(configFile, pt);
            syslog(LOG_INFO, "read ini");
    
            domainPath = pt.get<string>("common.domain-path", domainPath);
            syslog(LOG_INFO, "got path");
            magicClose = pt.get<bool>("client.magic-close", magicClose);
            watchdogTimeout = pt.get<int>("client.watchdog-timeout", watchdogTimeout);
            auto timeout = pt.get<int>("client.connect-timeout", connectTimeout.count());
            connectTimeout = seconds{ timeout };

            timeout = pt.get<int>("client.heartbeat-timeout", heartbeatTimeout.count());
            heartbeatTimeout = seconds{ timeout };
        }
        catch(const exception& ex) {
            throw runtime_error(string{"ini parse error: "} + ex.what());
        }

        auto exReporter = [](const string& msg, const exception& ex) {
            stringstream logstr;
            logstr << msg << ": " << ex.what() << endl;
            syslog(LOG_ERR, "IoService: %s", logstr.str().c_str());
        };

        auto logger = [](IoLog&& log) {
            wstringstream logstr;
            logstr << move(log);
            string nstr(logstr.str().begin(), logstr.str().end());
            syslog(LOG_NOTICE, "IoService: %s", nstr.c_str());
        };

        IoLogConsumer::AttachLogger(move(logger));
        IoServices serviceInstance(move(exReporter));
        serviceInstance.Start();

        UnixConnectionManager impl(serviceInstance, connectTimeout, heartbeatTimeout);

#if __GNUC__>4
        atomic_store(&sigHandlerHandle, make_shared<SigHandler>(serviceInstance, impl, watchdogTimeout, magicClose));
#else
        sigHandlerHandle = make_shared<SigHandler>(serviceInstance, impl, watchdogTimeout, magicClose);
#endif
        impl.Start();

        impl.TryAddConnection(domainPath);

        atomic<bool> isConnected{};
        atomic<bool> isInError{};

        auto udpRx = [](shared_ptr<DgramListener> listener, int available) {
            syslog(LOG_NOTICE, "unexpectedUDP bytes: %d", available);

            try {
                stringstream epStr;
                auto ep = listener->GetPeerEndPoint();
                epStr << ":
            }
            catch(const exception& ex) {
                syslog(LOG_NOTICE, "RX err get end point: %s", ex.what());
            }
        };

        auto udpErr = [&isInError](shared_ptr<DgramListener> listner, const string& msg) {
            isInError.store(true);
            syslog(LOG_NOTICE, "UDP err: %s", msg.c_str());
        };

        auto sender = serviceInstance.UnboundDgramListener(move(udpRx), move(udpErr));

        auto connectCb = [&isConnected](shared_ptr<DgramListener> listener) {
            isConnected.store(true);
            stringstream epStr;
            auto ep = listener->GetPeerEndPoint();
            epStr << ep;
            syslog(LOG_NOTICE, "UDP connected: %s", epStr.str().c_str());
        };

        sender->AsyncConnect(move(connectCb), "192.168.7.1", 8784);
		// process status messages
#if __GNUC__>4
		while (atomic_load(&sigHandlerHandle))
#else
		while (sigHandlerHandle)
#endif
		{
            if (isConnected.load()) {
                string hello("hello world");
                sender->AsyncWrite(move(hello), false);
            }

            this_thread::sleep_for(seconds(5));
		}
	}
    catch (const exception& ex)
    {
        syslog(LOG_CRIT, "Unable to start %s", ex.what());
    }

	return 0;
}
