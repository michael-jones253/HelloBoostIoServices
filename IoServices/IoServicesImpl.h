//
//  IoServicesImpl.h
//  AsyncIo
//
//  Created by Michael Jones on 26/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef HelloAsio_IoServicesImpl_h
#define HelloAsio_IoServicesImpl_h

#include "Timer.h"
#include "PeriodicTimer.h"
#include "TcpServer.h"
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include "UnixServer.h"
#endif
#include "UdpListener.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <future>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <iostream>

namespace AsyncIo {
    class IoServicesImpl final {
		using IoExceptionCallback = std::function<void(const std::string& msg, const std::exception&)>;

		IoExceptionCallback _exceptionNotifier{};
		std::atomic<bool> _shouldRun{};
		std::promise<bool> _started{};
		boost::asio::io_service _ioService{};
		std::vector<TcpServer> _tcpServers{};
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
		std::vector<UnixServer> _unixServers{};
#endif
		std::future<bool> _serviceRunHandle{};
		boost::thread_group _threadPool{};
		std::vector<boost::thread*> _workerHandles{};
		std::atomic<int> _freeWorkerCount{};
		std::unordered_map<Timer, boost::asio::deadline_timer, TimerHasher> _oneShotTimers{};
		std::unordered_map<PeriodicTimer, boost::asio::deadline_timer, PeriodicTimerHasher> _periodicTimers{};
		std::map<std::shared_ptr<TcpPeerConnection>, std::shared_ptr<TcpPeerConnection>> _clientConnections{};
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
		std::map<std::shared_ptr<TcpDomainConnection>, std::shared_ptr<TcpDomainConnection>> _domainConnections{};
#endif
		std::map<std::shared_ptr<UdpListener>, std::shared_ptr<UdpListener>> _listeners{};
		std::mutex _mutex{};
        
    public:
		IoServicesImpl(IoExceptionCallback&& ioExceptionCb);
        
        ~IoServicesImpl();
        
        std::future<bool> Start();
        
		void AddTcpServer(int port, AcceptStreamCallback&& acceptsStream, ReadStreamCallback&& readSome);

		void StartTcpServer(int port);

		void StartTcpServer(int port, SecurityOptions&& security);

		void AsyncConnect(ConnectCallback&& connectCb, ErrorCallback&& errCb, std::string ipAddress, int port);
        
		void SendToAllServerConnections(const std::string& msg, bool nullTerminate);

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
		void AddUnixServer(const std::string& path, UnixAcceptStreamCallback&& acceptsStream, UnixReadStreamCallback&& readSome);

		void StartUnixServer(const std::string& path);

		void AsyncConnect(DomainConnectCallback&& connectCb, DomainErrorCallback&& errCb, const std::string& path);
#endif

		std::shared_ptr<UdpListener> BindDgramListener(std::string ipAddress, int port);
        
		std::shared_ptr<UdpListener> BindDgramListener(int port);

		std::shared_ptr<UdpListener> MakeUnboundUdpListener();

		void Stop();
        
        void RunWork(const std::function<void(void)>& work);

		void InitialisePeriodicTimer(int id, const::std::string& name);
        
		void InitialiseOneShotTimer(int id, const::std::string& name);

		void SetPeriodicTimer(PeriodicTimer id,
                              boost::posix_time::time_duration durationFromNow,
                              const std::function<void(PeriodicTimer id)>&& handler);

		void CancelPeriodicTimer(PeriodicTimer id);

		void ErrorHandler(std::shared_ptr<UdpListener> listener, boost::system::error_code ec);

    private:
        bool Run();
        void AddWorker();
        void RemoveWorker();
        void WorkerThread(boost::asio::io_service& ioService);
		void ErrorHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec);
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
		void ErrorHandler(std::shared_ptr<TcpDomainConnection> conn, boost::system::error_code ec);
#endif
	};
}

#endif
