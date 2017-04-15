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
#include <unordered_map>
#include <chrono>
#include <vector>
#include <future>
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
    IoServices& _services;
    string _path;
    unordered_map<shared_ptr<UnixStreamConnection>, shared_ptr<UnixStreamConnection>> _connections;
public:
    Impl(IoServices& services, const string& path) :
        _path(path),
        _services{ services },
        _connections{}
    {
    }

    ~Impl() {
    }

    void Start() {
        auto acceptor = [this](shared_ptr<UnixStreamConnection> conn) {
            syslog(LOG_NOTICE, "Connection accepted");
            _connections[conn] = conn;
        };

        auto clientReader = [](shared_ptr<UnixStreamConnection> conn, int avail) {
            syslog(LOG_NOTICE, "Unexpected Bytes available`%d", avail);
        };

        auto connError = [](const string& msg) {
            syslog(LOG_NOTICE, "connect failed: `%s", msg.c_str());
        };

        auto ioError = [this](shared_ptr<UnixStreamConnection> conn, const string& msg) {
            syslog(LOG_NOTICE, "client IO error: `%s", msg.c_str());
            _connections.erase(conn);
        };

        _services.AddUnixServer(_path, move(acceptor), move(clientReader));
        _services.StartUnixServer(_path);

        auto timeout = [this](PeriodicTimer id) {
            syslog(LOG_NOTICE, "heartbeat timer");
            for (auto& conn : _connections) {
                string heartbeat{"heartbeat"};
                try {
                    conn.second->AsyncWrite(move(heartbeat), true);
                }
                catch (const exception& ex) {
                    // If the exception is not caught the IoServicesImpl
                    // run loop catches it, however the timer seems to
                    // disappear. I suspect boost removes deadline timers
                    // that throw an exception in their handler.
                    syslog(LOG_NOTICE, "exception heartbeat timer write: %s", ex.what());
                }
            }
        };

        _services.InitialisePeriodicTimer(PeriodicTimer{99, "Heartbeat"});
        _services.SetPeriodicTimer(PeriodicTimer{99, "Heartbeat"},
                                    seconds(5),
                                    move(timeout));
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
        ("path", argopt::value<std::string>()->required(), "domain socket path")
        ("dest-udp", argopt::value<std::string>(), "domain socket path");

        argopt::variables_map args;
        auto parsed = argopt::command_line_parser(argc, argv).options(theOptions).run();

        argopt:store(parsed, args);

        string domainPath("/var/run/domain");
        string destUdp("255.255.255.255");

        if (args.count("path") > 0)
        {
            domainPath = args["path"].as<std::string>();
        }

        if (args.count("dest-udp") > 0)
        {
            destUdp = args["dest-udp"].as<std::string>();
        }

        openlog("unix-optimeye-server", LOG_NDELAY | LOG_PID, LOG_LOCAL1);

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

        Impl app(serviceInstance, domainPath);
        app.Start();

		// process status messages
        auto udpRx = [](shared_ptr<DgramListener> listener, int available) {
            syslog(LOG_NOTICE, "unexpectedUDP bytes: %d", available);
        };

        auto udpErr = [](shared_ptr<DgramListener> listner, const string& msg) {
            syslog(LOG_NOTICE, "UDP err: %s", msg.c_str());
        };

        auto listener = serviceInstance.UnboundDgramListener(move(udpRx), move(udpErr));
        listener->EnableBroadcast();
        listener->LaunchRead();
        IoEndPoint dest{ destUdp, 4343 };
		while (true)
		{
            string hello("hello world");
            cout << hello << endl;
            listener->AsyncSendTo(move(hello), destUdp, 4343, false);

            vector<uint8_t> msg{ 0x23, 0x4d, 0x4a };
            listener->AsyncSendTo(move(msg), dest);

            this_thread::sleep_for(seconds(5));
		}
	}
    catch (const exception& ex)
    {
        syslog(LOG_CRIT, "Unable to start %s", ex.what());
    }

	return 0;
}
