// UDP to USB server

#include "UnixConnectionManager.h"

#include <IoServices/IoLogConsumer.h>
#include <boost/program_options.hpp>

#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <sstream>
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

void term(int signum)
{
	::exit(0);
}


int main(int argc, char *argv[])
{
	setlogmask(LOG_UPTO(LOG_DEBUG));
	// set up signal action
	struct sigaction action{};
	action.sa_handler = term;
	sigaction(SIGHUP, &action, 0);

    try
	{
        argopt::options_description theOptions("Allowed args");
        theOptions.add_options()
        ("path", argopt::value<string>()->default_value("/var/run/domain"), "domain socket path")
        ("connect-timeout", argopt::value<int>()->default_value(5), "connect timeout");

        argopt::variables_map args;
        auto parsed = argopt::command_line_parser(argc, argv).options(theOptions).run();

        argopt:store(parsed, args);

        string domainPath;
        seconds connectTimeout;

        if (args.count("path") > 0)
        {
            domainPath = args["path"].as<string>();
        }

        if (args.count("connect-timeout") > 0)
        {
            auto timeout  = args["connect-timeout"].as<int>();
            connectTimeout = seconds(timeout);
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

        impl.Start();

        impl.TryAddConnection(domainPath);

		// process status messages
		while (true)
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
