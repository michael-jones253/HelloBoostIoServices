//
//  IoServicesImpl.cpp
//  AsyncIo
//
//  Created by Michael Jones on 26/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//
#include "stdafx.h"
#include "IoServicesImpl.h"
#include "TcpConnection.hpp"
#include "IoLogConsumer.h"

#include <stdio.h>
#include <chrono>
#include <stdexcept>
#include <thread>

using namespace boost;
using namespace boost::asio;

namespace AsyncIo {

	IoServicesImpl::IoServicesImpl(IoExceptionCallback&& ioExceptionCb) :
		_exceptionNotifier{ std::move(ioExceptionCb) }
	{

	}


	void IoServicesImpl::InitialisePeriodicTimer(int id, const::std::string& name) {
		auto key = PeriodicTimer{ id, name };
		_periodicTimers.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(key),
			std::forward_as_tuple(std::ref(_ioService)));
	}

	void IoServicesImpl::InitialiseOneShotTimer(int id, const::std::string& name) {
		auto key = Timer{ id, name };
		_oneShotTimers.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(key),
			std::forward_as_tuple(std::ref(_ioService)));

	}


	IoServicesImpl::~IoServicesImpl() {
		try
		{
			Stop();
		}
		catch (const std::exception& ex)
		{
			_exceptionNotifier("Exception stopping IO service: ", ex);
		}
	}

	std::future<bool> IoServicesImpl::Start() {
		_shouldRun = true;

		auto res = _started.get_future();

		auto runLoop = [this]() ->bool { return Run(); };

		_serviceRunHandle = std::async(std::launch::async, std::move(runLoop));

		auto notifyStarted = [this]()->void {
			try {
				--_freeWorkerCount;
				_started.set_value(true);
			}
			catch (const std::exception& ex) {
				std::cerr << "Exception in start notification: " << ex.what() << std::endl;
			}

			_freeWorkerCount++;
		};

		_ioService.post(std::move(notifyStarted));
		return res;
	}

	void IoServicesImpl::AddTcpServer(int port, AcceptStreamCallback&& acceptsStream, ReadStreamCallback&& readSome) {
		auto predicate = [this, port](const TcpServer& it) {
			return it.GetPort() == port;
		};

		if (std::find_if(_tcpServers.begin(), _tcpServers.end(), predicate) != _tcpServers.end()) {

			throw std::runtime_error("Server already on port");
		}

		_tcpServers.emplace_back(&_ioService, port, std::move(acceptsStream), std::move(readSome));
	}

	void IoServicesImpl::StartTcpServer(int port)
	{
		auto predicate = [this, port](const TcpServer& it) {
			return it.GetPort() == port;
		};

		auto server = std::find_if(_tcpServers.begin(), _tcpServers.end(), predicate);
		if (server == _tcpServers.end()) {
			std::stringstream errStr;
			errStr << "No server for port: " << port;
			throw std::runtime_error(errStr.str());
		}

		server->Start();
	}

	void IoServicesImpl::StartTcpServer(int port, SecurityOptions&& security) {
		auto predicate = [this, port](const TcpServer& it) {
			return it.GetPort() == port;
		};

		auto server = std::find_if(_tcpServers.begin(), _tcpServers.end(), predicate);
		if (server == _tcpServers.end()) {
			std::stringstream errStr;
			errStr << "No secure server for port: " << port;
			throw std::runtime_error(errStr.str());
		}

		server->Start(std::move(security));
	}


	void IoServicesImpl::AsyncConnect(ConnectCallback&& connectCb, ErrorCallback&& errCb, std::string ipAddress, int port)
	{
		auto errCbCopy = std::move(errCb);
		auto errHandler = [this, errCbCopy](std::shared_ptr<TcpPeerConnection> conn, const boost::system::error_code& ec) {
			// Call the application error handler first.
			errCbCopy(conn, ec);

			// Then call our cleanup handler.
			ErrorHandler(conn, ec);
		};

		auto conn = std::make_shared<TcpConnection>(&_ioService, std::move(errHandler));

		// All access is from the context of the IO service so should not need mutexing.
		_clientConnections[conn] = conn;

		conn->AsyncConnect(std::move(connectCb), ipAddress, port);
	}

	void IoServicesImpl::ErrorHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec) {
		// All access is from the context of the IO service so should not need mutexing.
		if (_clientConnections.find(conn) == _clientConnections.end())
		{
			// Not expecting this to happen, but if it does the main loop will catch it and log.
			throw std::runtime_error("Connection not found");
		}

		_clientConnections.erase(conn);
	}

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
	void IoServicesImpl::ErrorHandler(std::shared_ptr<TcpDomainConnection> conn, boost::system::error_code ec) {
		// All access is from the context of the IO service so should not need mutexing.
		if (_domainConnections.find(conn) == _domainConnections.end())
		{
			// Not expecting this to happen, but if it does the main loop will catch it and log.
			throw std::runtime_error("Connection not found");
		}

		conn->Close();
		_domainConnections.erase(conn);
	}
#endif

	void IoServicesImpl::ErrorHandler(std::shared_ptr<UdpListener> listener, boost::system::error_code ec) {
		std::lock_guard<std::mutex> listenerGuard(_mutex);
		if (_listeners.find(listener) == _listeners.end())
		{
			// Not expecting this to happen, but if it does the main loop will catch it and log.
			throw std::runtime_error("Listener not found");
		}

		_listeners.erase(listener);
	}


	void IoServicesImpl::SendToAllServerConnections(const std::string& msg, bool nullTerminate) {
		for (auto& server : _tcpServers) {
			server.SendMessageToAllPeers(msg, nullTerminate);
		}
	}

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

	void IoServicesImpl::AddUnixServer(const std::string& path, UnixAcceptStreamCallback&& acceptsStream, UnixReadStreamCallback&& readSome) {
		auto predicate = [this, path](const UnixServer& it) {
			return it.GetPath() == path;
		};

		if (std::find_if(_unixServers.begin(), _unixServers.end(), predicate) != _unixServers.end()) {

			throw std::runtime_error("Server already on port");
		}

		_unixServers.emplace_back(&_ioService, path, std::move(acceptsStream), std::move(readSome));
	}

	void IoServicesImpl::StartUnixServer(const std::string& path)
	{
		auto predicate = [this, path](const UnixServer& it) {
			return it.GetPath() == path;
		};

		auto server = std::find_if(_unixServers.begin(), _unixServers.end(), predicate);
		if (server == _unixServers.end()) {
			std::stringstream errStr;
			errStr << "No server for path: " << path;
			throw std::runtime_error(errStr.str());
		}

		server->Start();
	}
	void IoServicesImpl::AsyncConnect(DomainConnectCallback&& connectCb, DomainErrorCallback&& errCb, const std::string& path) {
		auto errCbCopy = std::move(errCb);
		auto errHandler = [this, errCbCopy](std::shared_ptr<TcpDomainConnection> conn, const boost::system::error_code& ec) {
			// Call the application error handler first.
			errCbCopy(conn, ec);

			// Then call our cleanup handler.
			ErrorHandler(conn, ec);
		};

		auto conn = std::make_shared<TcpDomainConnection>(&_ioService, std::move(errHandler));

		// All access is from the context of the IO service so should not need mutexing.
		_domainConnections[conn] = conn;

		conn->AsyncConnect(std::move(connectCb), path);
	}
#endif

	std::shared_ptr<UdpListener> IoServicesImpl::BindDgramListener(std::string ipAddress, int port) {
		// Bind to specific ip interface address for receiving on that interface only.
		boost::asio::ip::address boostAddress = boost::asio::ip::address::from_string(ipAddress);

		auto listener = std::make_shared<UdpListener>(&_ioService, boostAddress, port);
		std::lock_guard<std::mutex> listenerGuard(_mutex);
		_listeners[listener] = listener;

		return listener;
	}

	std::shared_ptr<UdpListener> IoServicesImpl::BindDgramListener(int port) {
		auto listener = std::make_shared<UdpListener>(&_ioService, port);
		std::lock_guard<std::mutex> listenerGuard(_mutex);
		_listeners[listener] = listener;

		return listener;
	}

	std::shared_ptr<UdpListener> IoServicesImpl::MakeUnboundUdpListener() {
		auto listener = std::make_shared<UdpListener>(&_ioService);
		std::lock_guard<std::mutex> listenerGuard(_mutex);
		_listeners[listener] = listener;

		return listener;
	}

	void IoServicesImpl::Stop() {
		if (!_serviceRunHandle.valid())
		{
			return;
		}

		_shouldRun = false;
		_ioService.stop();
		_threadPool.join_all();
		auto ok = _serviceRunHandle.get();
		if (ok) {
			// std::cout << "Io Service ran OK" << std::endl;
		}
	}

	bool IoServicesImpl::Run() {
		io_service::work keepRunning(_ioService);

		system::error_code ec;

		while (_shouldRun.load())
		{
			try
			{
				_ioService.run(ec);
			}
			catch (const std::exception& ex)
			{
				_exceptionNotifier("Exception in IO service: ", ex);
			}
		}

		LOG() << "Stopping IO service." << std::endl;

		return true;
	}

	void IoServicesImpl::RunWork(const std::function<void(void)>& work) {
		// Make as many workers as needed to support the concurrent load.
		if (_freeWorkerCount == 0) {
			AddWorker();
		}

		// We will take a copy of work.
		auto workWrapper = [this, work]()->void {
			try {
				--_freeWorkerCount;
				work();
			}
			catch (const std::exception& ex) {
				_exceptionNotifier("Exception in work: ", ex);
			}

			_freeWorkerCount++;
		};

		// MJ Review move.
		_ioService.post(std::move(workWrapper));
	}

	void IoServicesImpl::SetPeriodicTimer(
		PeriodicTimer id,
		boost::posix_time::time_duration durationFromNow,
		const std::function<void(PeriodicTimer id)>&& handler) {
		// move fn directly into lambda capture - c14
#if defined(__LINUX__)
		auto resetHandler = [this, id, durationFromNow, handlerCopy(std::move(handler))](boost::system::error_code ec) {
#else
		auto handlerCopy = std::move(handler);
		auto resetHandler = [this, id, durationFromNow, handlerCopy](boost::system::error_code ec) {
#endif
			if (ec == boost::asio::error::operation_aborted)
			{
				// User cancelled.
				return;
			}

			// invoke it before moving
			handlerCopy(id);

			// Chain the next periodic timeout.
			this->SetPeriodicTimer(id, durationFromNow, std::move(handlerCopy));
		};

		auto timer = _periodicTimers.find(id);
		if (timer != _periodicTimers.end()) {
			timer->second.expires_from_now(durationFromNow);
			timer->second.async_wait(std::move(resetHandler));
		}
	}

	void IoServicesImpl::CancelPeriodicTimer(PeriodicTimer id) {
		auto timer = _periodicTimers.find(id);
		// Review: throw if not found.
		if (timer != _periodicTimers.end()) {
			timer->second.cancel();
		}
	}

	void IoServicesImpl::AddWorker() {
		auto work = std::bind(&IoServicesImpl::WorkerThread, this, std::ref(_ioService));
		auto worker = _threadPool.create_thread(std::move(work));
		_workerHandles.push_back(worker);
		++_freeWorkerCount;
	}

	void IoServicesImpl::RemoveWorker() {
		_threadPool.remove_thread(_workerHandles.back());
		_workerHandles.pop_back();
	}

	void IoServicesImpl::WorkerThread(boost::asio::io_service& ioService) {
		// std::cout << "hello worker" << std::endl;
		ioService.run();
		// std::cout << "Goodbye worker" << std::endl;
	}

}
