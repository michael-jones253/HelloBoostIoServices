// UDP to USB server

#include <IoServices/IoServices.h>
#include <IoServices/IoLogConsumer.h>

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
#include <future>
#include <stdexcept>
#include <errno.h>
#include <syslog.h>

using namespace AsyncIo;
using namespace std;
using namespace std::chrono;


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
        if (argc < 2)
        {
            throw runtime_error("usage <program> <domain path>");
        }

        string domainPath = argv[1];

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

        promise<shared_ptr<UnixStreamConnection>> connPromise{};
        auto connFuture = connPromise.get_future();

        auto connector = [&connPromise](shared_ptr<UnixStreamConnection> conn) {
            syslog(LOG_NOTICE, "Connection accepted");
            connPromise.set_value(conn);
        };

        auto clientReader = [](shared_ptr<UnixStreamConnection> conn, int avail) {
            syslog(LOG_NOTICE, "Bytes available`%d", avail);
        };

        auto connError = [&connPromise](const string& msg) {
            syslog(LOG_NOTICE, "connect failed: `%s", msg.c_str());
            connPromise.set_value(nullptr);
        };

        auto ioError = [](shared_ptr<UnixStreamConnection> conn, const string& msg) {
            syslog(LOG_NOTICE, "client IO error: `%s", msg.c_str());
        };

        serviceInstance.AsyncConnect(move(connector), move(clientReader), move(connError), move(ioError), domainPath);

        auto connStatus =connFuture.wait_for(seconds(5));
        if (connStatus == future_status::timeout)
        {
            throw runtime_error("Connection timed out to " + domainPath);
        }

        shared_ptr<UnixStreamConnection> clientConn{};
        clientConn = connFuture.get();

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
