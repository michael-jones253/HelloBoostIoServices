//
//  IoServicesImpl.cpp
//  HelloAsio
//
//  Created by Michael Jones on 26/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoServicesImpl.h"

#include <stdio.h>

using namespace boost;
using namespace boost::asio;

namespace HelloAsio {
    
    IoServicesImpl::~IoServicesImpl() {
        
    }
    
    void IoServicesImpl::Start() {
        
        _serviceRunHandle = std::async(std::launch::async, [this]() ->bool { return Run(); });
        
        auto work = std::bind(&IoServicesImpl::WorkerThread, this, std::ref(_ioService) );
        _threadPool.create_thread(std::move(work));
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
    
    void IoServicesImpl::RunWork(std::function<void(void)> work) {
        _ioService.post(work);
    }
    
    void IoServicesImpl::WorkerThread(boost::asio::io_service& ioService) {
        std::cout << "hello worker" << std::endl;
        ioService.run();
        std::cout << "Goodbye worker" << std::endl;
    }
    

}
