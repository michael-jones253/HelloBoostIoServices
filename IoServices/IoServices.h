/*
 *  IoServices.h
 *  IoServices
 *
 *  Created by Michael Jones on 26/07/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#ifndef IoServices_
#define IoServices_

#include "PeriodicTimer.h"
#include "StreamConnection.h"
#include <memory>
#include <functional>
#include <chrono>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace HelloAsio {
    class IoServicesImpl;
    
    class IoServices final {
        std::unique_ptr<IoServicesImpl> _impl;
        ReadStreamCallback _readStream;
        StreamErrorCallback _error;
        
    public:
        IoServices();
        ~IoServices();
        
        void RunTcpServer(int port, ReadStreamCallback&& readStream);
        
        void HelloAllPeers();
        
        void RunWork(std::function<void(void)>const& work);
        void SetPeriodicTimer(
                              PeriodicTimer id,
                              std::chrono::duration<long long>  du,
                              const std::function<void(PeriodicTimer id)>& handler);

    };
    
}

#pragma GCC visibility pop
#endif
