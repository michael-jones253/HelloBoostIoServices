//
//  IoServicesImpl.h
//  HelloAsio
//
//  Created by Michael Jones on 26/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef HelloAsio_IoServicesImpl_h
#define HelloAsio_IoServicesImpl_h

#include <future>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>

#pragma GCC visibility push(hidden)

namespace HelloAsio {
    class IoServicesImpl final {
        boost::asio::io_service _ioService;
        std::future<bool> _serviceRunHandle;
        boost::thread_group _threadPool;
        
    public:
        IoServicesImpl() {};
        
        ~IoServicesImpl();
        
        void Start();
        
        void Stop();
        
        void RunWork(std::function<void(void)> work);
        
    private:
        bool Run();
        
        void WorkerThread(boost::asio::io_service& ioService);
    };
}

#pragma GCC visibility pop

#endif
