// UDP to USB server

#include "UnixConnectionManager.h"

#include <IoServices/IoLogConsumer.h>
#include <boost/program_options.hpp>

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
    SigHandler(IoServices& services, UnixConnectionManager& mgr, int timeout) :
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
            _magicClose = true;
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
        _shouldRun.store(false);
        _mgr.Stop();
        _services.Stop();

        if (true || _magicClose) {
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

	auto handle = atomic_load(&sigHandlerHandle);
    if (handle) {
        sigHandlerHandle.reset();
    }
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
        ("path", argopt::value<string>()->default_value("/var/run/domain"), "domain socket path")
        ("connect-timeout", argopt::value<int>()->default_value(5), "connect timeout")
        ("watchdog-timeout", argopt::value<int>()->default_value(5), "connect timeout");

        argopt::variables_map args;
        auto parsed = argopt::command_line_parser(argc, argv).options(theOptions).run();

        argopt:store(parsed, args);

        string domainPath;
        seconds connectTimeout;
        int watchdogTimeout;

        if (args.count("path") > 0)
        {
            domainPath = args["path"].as<string>();
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

        openlog("unix-optimeye-client", LOG_NDELAY | LOG_PID, LOG_LOCAL1);

        auto path = "/etc/catapult/optimeye.ini";
        ifstream configFile(path);
        if (!configFile.good())
        {
            throw runtime_error(string("Master service failed to load: ") + path);
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

        UnixConnectionManager impl(serviceInstance, connectTimeout);

        atomic_store(&sigHandlerHandle, make_shared<SigHandler>(serviceInstance, impl, watchdogTimeout));
        impl.Start();

        impl.TryAddConnection(domainPath);

		// process status messages
		while (atomic_load(&sigHandlerHandle))
		{
            this_thread::sleep_for(seconds(5));
		}
	}
    catch (const exception& ex)
    {
        syslog(LOG_CRIT, "Unable to start %s", ex.what());
    }

	return 0;
}
