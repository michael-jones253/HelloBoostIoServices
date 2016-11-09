//
//  UnixConnectionManager.cpp
//  AsyncIo
//
//  Created by Michael Jones on 08/11/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#include "UnixConnectionManager.h"
#include "ClientUnixConnection.h"
#include <map>
#include <mutex>
#include <syslog.h>

using namespace AsyncIo;
using namespace std;
using namespace std::chrono;

namespace UnixClient
{
    class UnixConnectionManagerImpl;

	/// <summary>
	/// For async IO with client connections.
	/// NB this class is managed internally by the IO services and not to be instantiated by application code.
	/// </summary>
    class UnixConnectionManagerImpl final
	{
    private:
        mutex _mutex;
        IoServices& _ioService;
        seconds _timeout;
		map<string, ClientUnixConnection> _connections;

    public:
		UnixConnectionManagerImpl() = delete;

		/// <summary>
		/// For async IO with client connections.
		/// NB this class is managed internally by the IO services and not to be instantiated by application code.
		/// </summary>
		/// <param name="ioService">The boost IO service.</param>
		/// <param name="timeout">Client IO timeout.</param>
		UnixConnectionManagerImpl(IoServices& ioService, seconds timeout) :
            _ioService{ ioService },
            _timeout{ timeout }
        {
        }

        UnixConnectionManagerImpl(UnixConnectionManager&& rhs);
		UnixConnectionManagerImpl& operator=(UnixConnectionManagerImpl&& rhs);
		UnixConnectionManagerImpl(const UnixConnectionManagerImpl&) = delete;
		UnixConnectionManagerImpl& operator=(const UnixConnectionManagerImpl&) = delete;

		~UnixConnectionManagerImpl() {
        }

        void Start() {
        }

        void Stop() {
        }

        bool TryAddConnection(const string& path) {
            lock_guard<mutex> guard(_mutex);
            auto found = _connections.find(path);
            if (found != _connections.end()) {
                return false;
            }

            auto connect = [this](const string& path, shared_ptr<UnixStreamConnection> conn) {
                lock_guard<mutex> guard(_mutex);
                auto found = _connections.find(path);
                if (found == _connections.end()) {
                    syslog(LOG_NOTICE, "connect for expired client");
                    return false;
                }

                found->second.SetConnection(path, conn);
            };

            auto error = [this](const string& path, shared_ptr<UnixStreamConnection> conn, const string& msg) {
                lock_guard<mutex> guard(_mutex);
                auto found = _connections.find(path);
                if (found == _connections.end()) {
                    syslog(LOG_NOTICE, "error for expired client");
                    return false;
                }

                found->second.SetError(path, conn, msg);
            };

            auto client = _connections.emplace(piecewise_construct, forward_as_tuple(path), forward_as_tuple(_ioService, path, _timeout, move(connect), move(error)));

            if (client.second) {
                client.first->second.Start();
            }

            return true;
        }
    };

    UnixConnectionManager::UnixConnectionManager(IoServices& ioService, seconds timeout) : _impl{ make_unique<UnixConnectionManagerImpl>(ioService, timeout) } {
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
        _impl->TryAddConnection(path);
    }
}

