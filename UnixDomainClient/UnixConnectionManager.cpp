//
//  UnixConnectionManager.h
//  AsyncIo
//
//  Created by Michael Jones on 08/11/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#include "UnixConnectionManager.h"
#include "ClientUnixConnection.h"

#include <unordered_map>
#include <mutex>
#include <future>
#include <syslog.h>

using namespace std;
using namespace std::chrono;
using namespace AsyncIo;

namespace UnixClient
{
    class UnixConnectionManagerImpl {
    private:
        mutex _mutex{};
        unordered_map<string, ClientUnixConnection> _connections{};
        IoServices& _services;
        seconds _timeout;

    public:
		UnixConnectionManagerImpl() = delete;

		/// <summary>
		/// For async IO with client connections.
		/// NB this class is managed internally by the IO services and not to be instantiated by application code.
		/// </summary>
		/// <param name="ioService">The boost IO service.</param>
		/// <param name="path">The path to listen on.</param>
		/// <param name="acceptsStream">Client connection accepted callback.</param>
		/// <param name="readSomeCb">Client read some data callback.</param>
		UnixConnectionManagerImpl(IoServices& ioService, seconds timeout) :
            _services{ ioService },
            _timeout{ timeout }
        {
        }

		~UnixConnectionManagerImpl() {
        }

        void Start() {
        auto timeout = [this](PeriodicTimer id) {
            syslog(LOG_NOTICE, "timeout");
            auto timeNow = system_clock::now();
            for (auto& conn : _connections) {
                if (timeNow - conn.second.GetTimeLastHeartbeat() > seconds(10) ) {
                    syslog(LOG_NOTICE, "heartbeat lost");
                }
            }
        };

        _services.InitialisePeriodicTimer(PeriodicTimer{99, "Heartbeat"});
        _services.SetPeriodicTimer(PeriodicTimer{99, "Heartbeat"},
                                    seconds(5),
                                    move(timeout));
        }

        void Stop() {
        }

        bool TryAddConnection(const string& path) {
            lock_guard<mutex> guard(_mutex);
            auto con = _connections.find(path);
            if (con != _connections.end()) {
                syslog(LOG_INFO, "Already have a client for this path %s", path.c_str());
                return false;
            }

            auto connCb = [this](const string& path, shared_ptr<UnixStreamConnection> conn) {
                HandleConnectCallback(path, conn);
            };

            auto errCb = [this](const string& path, shared_ptr<UnixStreamConnection> conn, const string& msg) {
                HandleErrorCallback(path, conn, msg);
            };

            auto  client = _connections.emplace(piecewise_construct,
                forward_as_tuple(path),
                forward_as_tuple(_services, path, _timeout, move(connCb), move(errCb)));

            if (client.second) {
                client.first->second.Start();
            }

            return true;
        }

    private:
    void HandleConnectCallback(const string& path, shared_ptr<UnixStreamConnection> newConn) {
        lock_guard<mutex> guard(_mutex);
        auto client = _connections.find(path);
        if (client == _connections.end()) {
            syslog(LOG_INFO, "connect for expired client");
            return;
        }

        client->second.SetConnection(path, newConn);
    }

    void HandleErrorCallback(const string& path, shared_ptr<UnixStreamConnection> conn, const string& msg) {
        lock_guard<mutex> guard(_mutex);
        auto client = _connections.find(path);
        if (client == _connections.end()) {
            syslog(LOG_INFO, "error for expired client");
            return;
        }

        client->second.SetError(path, conn, msg);
    }

    };

    UnixConnectionManager::UnixConnectionManager(IoServices& ioService, seconds timeout) :
        _impl{ make_unique<UnixConnectionManagerImpl>(ioService, timeout) }
    {
    }

    UnixConnectionManager::~UnixConnectionManager() {
    }

    void UnixConnectionManager::Start() {
        _impl->Start();
    }

    void UnixConnectionManager::Stop() {
        _impl->Stop();
    }

    bool UnixConnectionManager::TryAddConnection(const string& path) {
        return _impl->TryAddConnection(path);
    }
}
