#include "ClientUnixConnection.h"

#include <IoServices/IoServices.h>
#include <IoServices/IoLogConsumer.h>

#include <future>
#include <mutex>
#include <condition_variable>
#include <syslog.h>

using namespace AsyncIo;
using namespace std;
using namespace std::chrono;

namespace UnixClient
{
    class ClientUnixConnectionImpl {
    private:
        // predicate
        bool _shouldRun{};
        bool _ioError{};
        condition_variable _condition{};
        mutex _mutex{};

        // non predicate
        atomic<bool> mExpired{};
        shared_ptr<UnixStreamConnection> _clientConn{};
        promise<shared_ptr<UnixStreamConnection>> mConnPromise{};
        promise<bool> mErrPromise{};
        future<bool> mTaskHandle{};
        IoServices& mIoService;
        string mDomainPath;
        seconds mTimeout;
        system_clock::time_point _lastHeartbeat{};

        ConnectCallback _ConnectCallback;
        ErrorCallback mErrorCallback;
    public:
		ClientUnixConnectionImpl(AsyncIo::IoServices& ioService, const std::string& path, seconds timeout, ConnectCallback&& connCb, ErrorCallback&& errCb) :
            mIoService(ioService),
            mDomainPath(path),
            mTimeout{ timeout },
            _ConnectCallback{ move(connCb) },
            mErrorCallback{ move(errCb) }
        {
        }
    
		~ClientUnixConnectionImpl() {
            try {
                Stop();
            }
            catch(const exception& ex) {
                syslog(LOG_NOTICE, "Client shutdown exception%s", ex.what());
            }
        }

        void Start() {
            _shouldRun = true;
            auto run = [this]() ->bool {
                return Work();
            };
    
            mTaskHandle = async(launch::async, move(run));
        }

        void Stop() {
            {
                lock_guard<mutex> gd(_mutex);
                _shouldRun = false;
            }

            _condition.notify_one();

            auto ok = mTaskHandle.get();

            if (!ok) {
                syslog(LOG_NOTICE, "Client shutdown with error");
            }
        }

        bool Expired() const {
            return mExpired.load();
        }
    

        void SetConnection(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection> conn) {
            auto myConn = atomic_load(&_clientConn);
            if (myConn) {
                syslog(LOG_NOTICE, "ignoring connect for connected");
                return;
            }

            if (path != mDomainPath) {
                syslog(LOG_NOTICE, "ignoring connect for different path");
                return;
            }

            syslog(LOG_NOTICE, "setting connection promise %s", path.c_str());
            mConnPromise.set_value(conn);
        }

        void SetError(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection> conn, const std::string& msg) {
            auto myCon = atomic_load(&_clientConn);
            if (!myCon) {
                syslog(LOG_NOTICE, "ignoring error for not connected %s", msg.c_str());
                return;
            }

            if (myCon != conn) {
                syslog(LOG_NOTICE, "ignoring error for different connection %s", msg.c_str());
                return;
            }


            if (path != mDomainPath) {
                syslog(LOG_NOTICE, "ignoring error for different path %s", msg.c_str());
                return;
            }

            syslog(LOG_NOTICE, "setting error promise %s", msg.c_str());
            mErrPromise.set_value(true);
            {
                lock_guard<mutex> gd(_mutex);
                _ioError = true;
            }

            _condition.notify_one();
        }

        system_clock::time_point GetTimeLastHeartbeat() const {
            return _lastHeartbeat;
        }
    private:
        bool RunOnce() {
            auto connFuture = mConnPromise.get_future();
            auto errFuture = mErrPromise.get_future();
    
            auto path = mDomainPath;
            auto connCb = _ConnectCallback;
            // No member capture - take copies
            auto connector = [cb{ move(connCb) }, domain(move(path))](shared_ptr<UnixStreamConnection> conn) {
                cb(domain, conn);
            };
    
            auto clientReader = [this](shared_ptr<UnixStreamConnection> conn, int avail) {
                syslog(LOG_NOTICE, "Bytes available`%d", avail);
                _lastHeartbeat = system_clock::now();
                conn->Consume(avail);
            };
    
            // No member capture
            auto connError = [](const string& msg) {
                syslog(LOG_NOTICE, "client async connect error : `%s", msg.c_str());
            };
    
            auto domainPath = mDomainPath;
            auto errCb = mErrorCallback;
            auto ioError = [cb{ move(errCb) }, path{ move(domainPath) }](shared_ptr<UnixStreamConnection> conn, const string& msg) {
                cb(path, conn, msg);
            };
    
            syslog(LOG_NOTICE, "client async connecting : `%s", mDomainPath.c_str());
            mIoService.AsyncConnect(move(connector), move(clientReader), move(connError), move(ioError), mDomainPath);
    
            auto connStatus = connFuture.wait_for(mTimeout);
            if (connStatus == future_status::timeout)
            {
                // No connection handle to close, can throw.
                throw runtime_error("Connection timed out to " + mDomainPath);
            }
    
            atomic_store(&_clientConn, connFuture.get());
    
            auto predicate = [this]() {
                return !_shouldRun || _ioError;
            };
    
            unique_lock<mutex> lk(_mutex);
            _condition.wait(lk, move(predicate));

            if (!_ioError) {
                auto conn = atomic_load(&_clientConn);
                if (conn) {
                    conn->Close();
                }
            }

            auto errStatus = errFuture.wait_for(mTimeout);
            return errStatus != future_status::timeout;
        }
    
        bool Work() {
            bool ok{};
            try {
                ok = RunOnce();
            } catch (const exception& ex) {
                syslog(LOG_NOTICE, "client run error: %s", ex.what());
            }

            syslog(LOG_NOTICE, "Client stopped %s", (ok ? "normally": "with error"));
            mExpired.store(true);

            return ok;
        }
    };

    ClientUnixConnection::ClientUnixConnection(IoServices& ioService, const string& path, seconds timeout, ConnectCallback&& connCb, ErrorCallback&& errCb) :
        _impl{ make_unique<ClientUnixConnectionImpl>(ioService, path, timeout, move(connCb), move(errCb)) }
    {
    }

    ClientUnixConnection::~ClientUnixConnection() {
    }

    void ClientUnixConnection::Start() {
        _impl->Start();
    }

    void ClientUnixConnection::Stop() {
        _impl->Stop();
    }

    bool ClientUnixConnection::Expired() const {
        return _impl->Expired();
    }

    void ClientUnixConnection::SetConnection(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection> conn) {
        _impl->SetConnection(path, conn);
    }

    void ClientUnixConnection::SetError(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection> conn, const std::string& msg) {
        _impl->SetError(path, conn, msg);
    }

    system_clock::time_point ClientUnixConnection::GetTimeLastHeartbeat() const {
        return _impl->GetTimeLastHeartbeat();
    }
}
