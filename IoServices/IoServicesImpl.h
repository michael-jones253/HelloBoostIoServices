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
#include "UnixServer.h"
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
		std::vector<UnixServer> _unixServers{};
		std::future<bool> _serviceRunHandle{};
		boost::thread_group _threadPool{};
		std::vector<boost::thread*> _workerHandles{};
		std::atomic<int> _freeWorkerCount{};
		std::unordered_map<Timer, boost::asio::deadline_timer, TimerHasher> _oneShotTimers{};
		std::unordered_map<PeriodicTimer, boost::asio::deadline_timer, PeriodicTimerHasher> _periodicTimers{};
		std::map<std::shared_ptr<TcpPeerConnection>, std::shared_ptr<TcpPeerConnection>> _clientConnections{};
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

		void AddUnixServer(const std::string& path, UnixAcceptStreamCallback&& acceptsStream, UnixReadStreamCallback&& readSome);

		void StartUnixServer(const std::string& path);

		std::shared_ptr<UdpListener> BindDgramListener(std::string ipAddress, int port);
        
		std::shared_ptr<UdpListener> BindDgramListener(int port);

		std::shared_ptr<UdpListener> MakeUnboundUdpListener();

		void Stop();
        
        void RunWork(const std::function<void(void)>& work);

		void InitialisePeriodicTimer(int id, const::std::string& name);
        
		void InitialiseOneShotTimer(int id, const::std::string& name);

		void SetPeriodicTimer(PeriodicTimer id,
                              boost::posix_time::time_duration durationFromNow,
                              const std::function<void(PeriodicTimer id)>& handler);

		void CancelPeriodicTimer(PeriodicTimer id);

		void ErrorHandler(std::shared_ptr<UdpListener> listener, boost::system::error_code ec);

    private:
        bool Run();
        void AddWorker();
        void RemoveWorker();
        void WorkerThread(boost::asio::io_service& ioService);
		void ErrorHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec);
	};
}

#endif
