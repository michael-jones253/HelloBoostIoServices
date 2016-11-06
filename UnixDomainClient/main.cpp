// UDP to USB server

#include <IoServices/IoServices.h>
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
#include <future>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <errno.h>
#include <syslog.h>

using namespace AsyncIo;
using namespace std;
using namespace std::chrono;

namespace argopt = boost::program_options;

void term(int signum)
{
	::exit(0);
}


class Impl {
private:
    bool mShouldRun{};
    condition_variable mCondition{};
    mutex mMutex{};
    shared_ptr<UnixStreamConnection> mClientConn{};
    future<bool> mTaskHandle{};
    IoServices& mIoService;
    string mDomainPath;
    seconds mTimeout;
public:
    Impl(IoServices& service, const string& path, seconds timeout) :
        mIoService(service),
        mDomainPath(path),
        mTimeout{ timeout }
    {
    }

    void Start() {
        mShouldRun = true;
        auto run = [this]() ->bool {
            WorkLoop();
            return true;
        };

        mTaskHandle = async(launch::async, move(run));
    }

private:
    void RunOnce() {
        promise<shared_ptr<UnixStreamConnection>> connPromise{};
        auto connFuture = connPromise.get_future();

        auto connector = [&connPromise](shared_ptr<UnixStreamConnection> conn) {
            syslog(LOG_NOTICE, "Connection accepted");
            connPromise.set_value(conn);
        };

        auto clientReader = [](shared_ptr<UnixStreamConnection> conn, int avail) {
            syslog(LOG_NOTICE, "Bytes available`%d", avail);
        };

        // NB do not pass in non statics to this callback
        auto connError = [](const string& msg) {
            syslog(LOG_NOTICE, "connect failed: `%s", msg.c_str());
        };

        auto ioError = [](shared_ptr<UnixStreamConnection> conn, const string& msg) {
            syslog(LOG_NOTICE, "client IO error: `%s", msg.c_str());
        };

        syslog(LOG_NOTICE, "client connecting : `%s", mDomainPath.c_str());
        mIoService.AsyncConnect(move(connector), move(clientReader), move(connError), move(ioError), mDomainPath);

        auto connStatus =connFuture.wait_for(mTimeout);
        if (connStatus == future_status::timeout)
        {
            throw runtime_error("Connection timed out to " + mDomainPath);
        }

        mClientConn = connFuture.get();

        auto predicate = [this]() {
            return !mShouldRun;
        };

        unique_lock<mutex> lk(mMutex);
        mCondition.wait(lk, move(predicate));
    }

    void WorkLoop() {
        while (mShouldRun) {
            try {
                RunOnce();
            } catch (const exception& ex) {
                this_thread::sleep_for(seconds(3));
            }
        }
    }
};

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

        Impl impl(serviceInstance, domainPath, connectTimeout);

        impl.Start();

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
