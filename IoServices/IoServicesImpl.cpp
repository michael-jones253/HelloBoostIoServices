//
//  IoServicesImpl.cpp
//  HelloAsio
//
//  Created by Michael Jones on 26/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoServicesImpl.h"

#include <stdio.h>
#include <chrono>

using namespace boost;
using namespace boost::asio;

namespace HelloAsio {
    
    IoServicesImpl::IoServicesImpl() {
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
        
    }
    
    void IoServicesImpl::Start() {
        
        _serviceRunHandle = std::async(std::launch::async, [this]() ->bool { return Run(); });
    
    }
    
    void IoServicesImpl::Stop() {
        _ioService.stop();
    }
    
    bool IoServicesImpl::Run() {
        io_service::work keepRunning(_ioService);
        
        system::error_code ec;
        _ioService.run(ec);
        
        std::cout << "Io Service ran OK" << std::endl;
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
        auto resetHandler = [this, id, durationFromNow, &handler](boost::system::error_code ec) {
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
