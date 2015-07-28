//
//  IoServicesImpl.h
//  HelloAsio
//
//  Created by Michael Jones on 26/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef HelloAsio_IoServicesImpl_h
#define HelloAsio_IoServicesImpl_h

#include "Timer.h"
#include "PeriodicTimer.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <future>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <iostream>

#pragma GCC visibility push(hidden)

namespace HelloAsio {
    class IoServicesImpl final {
        boost::asio::io_service _ioService;
        std::future<bool> _serviceRunHandle;
        boost::thread_group _threadPool;
        std::vector<boost::thread*> _workerHandles;
        std::atomic<int> _freeWorkerCount;
        std::unordered_map<Timer, boost::asio::deadline_timer> _oneShotTimers;
        std::unordered_map<PeriodicTimer, boost::asio::deadline_timer> _periodicTimers;
        
    public:
        IoServicesImpl();
        
        ~IoServicesImpl();
        
        void Start();
        
        void Stop();
        
        void RunWork(const std::function<void(void)>& work);
        
        void SetPeriodicTimer(PeriodicTimer id, const std::function<void(void)>& handler);
        
    private:
        bool Run();
        void AddWorker();
        void RemoveWorker();
        void WorkerThread(boost::asio::io_service& ioService);
    };
}

#pragma GCC visibility pop

#endif
