//
//  IoServicesImpl.cpp
//  HelloAsio
//
//  Created by Michael Jones on 26/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"
#include "IoServicesImpl.h"

#include <stdio.h>
#include <chrono>
#include <stdexcept>
#include <thread>

using namespace boost;
using namespace boost::asio;

namespace HelloAsio {
    
    IoServicesImpl::IoServicesImpl()
	 {
        for (Timer t{}; t < Timer::End; ++t) {
            _oneShotTimers.emplace(
                                   std::piecewise_construct,
                                   std::forward_as_tuple(t),
                                   std::forward_as_tuple(std::ref(_ioService)));
        }

        for (PeriodicTimer t{}; t < PeriodicTimer::End; ++t) {
            _periodicTimers.emplace(
                                   std::piecewise_construct,
                                   std::forward_as_tuple(t),
                                   std::forward_as_tuple(std::ref(_ioService)));
        }
    }
    
    IoServicesImpl::~IoServicesImpl() {
		try
		{
			Stop();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception stopping IO service: " << ex.what() << std::endl;
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
    
    void IoServicesImpl::RunTcpServer(int port, ReadSomeCallback&& readSome) {
        auto predicate = [this,port](const TcpServer& it) {
            return it.GetPort() == port;
        };
        
        if (std::find_if(_tcpServers.begin(), _tcpServers.end(), predicate) != _tcpServers.end()) {
            
            throw std::runtime_error("Server already on port");
        }
        
        _tcpServers.emplace_back(&_ioService, port, std::move(readSome));
        
        _tcpServers.back().Start();
    }

	void IoServicesImpl::AsyncConnect(ConnectCallback&& connectCb, std::string ipAddress, int port)
	{
		auto errorHandler = std::bind(&IoServicesImpl::ErrorHandler, this, std::placeholders::_1, std::placeholders::_2);

		// FIX ME put this connection in a list or map.
		auto conn = std::make_shared<TcpPeerConnection>(&_ioService, std::move(errorHandler));
		static int ConnectionIds{};
		_clientConnections[ConnectionIds++] = conn;

		conn->AsyncConnect(std::move(connectCb), ipAddress, port);
	}
    
	void IoServicesImpl::ErrorHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec) {
	}

    void IoServicesImpl::HelloAllPeers() {
        _tcpServers.front().SendMessageToAllPeers("Hello World!");
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
            std::cout << "Io Service ran OK" << std::endl;
        }
    }
    
    bool IoServicesImpl::Run() {
        io_service::work keepRunning(_ioService);
        
        system::error_code ec;
		std::cout << "RUN IOSERVICE" << std::endl;

		while (_shouldRun.load())
		{
			try
			{
				_ioService.run(ec);
			}
			catch (const std::exception& ex)
			{
				std::cout << "EXEPTION IO SERVICE: " << ex.what() << std::endl;
			}
		}

		std::cout << "STOPPING IOSERVICE" << std::endl;
        
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
            } catch (const std::exception& ex) {
                std::cerr << "Exception in work: " << ex.what() << std::endl;
            }
            
            _freeWorkerCount++;
        };
        
        // MJ Review move.
        _ioService.post(std::move(workWrapper));
    }
    
    void IoServicesImpl::SetPeriodicTimer(
                                          PeriodicTimer id,
                                          boost::posix_time::time_duration durationFromNow,
                                          const std::function<void(PeriodicTimer id)>& handler) {
        auto resetHandler = [this, id, durationFromNow, handler](boost::system::error_code ec) {
            handler(id);
            this->SetPeriodicTimer(id, durationFromNow, handler);
        };
        
        auto timer = _periodicTimers.find(id);
        if (timer != _periodicTimers.end()) {
            timer->second.expires_from_now(durationFromNow);
            timer->second.async_wait(std::move(resetHandler));
        }
    }

    
    void IoServicesImpl::AddWorker() {
        auto work = std::bind(&IoServicesImpl::WorkerThread, this, std::ref(_ioService) );
        auto worker = _threadPool.create_thread(std::move(work));
        _workerHandles.push_back(worker);
        ++_freeWorkerCount;
    }
    
    void IoServicesImpl::RemoveWorker() {
        _threadPool.remove_thread(_workerHandles.back());
        _workerHandles.pop_back();
    }
    
    void IoServicesImpl::WorkerThread(boost::asio::io_service& ioService) {
        std::cout << "hello worker" << std::endl;
        ioService.run();
        std::cout << "Goodbye worker" << std::endl;
    }

}
