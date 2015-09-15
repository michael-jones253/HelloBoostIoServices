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
#include "DgramListener.h"
#include <memory>
#include <functional>
#include <chrono>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace HelloAsio {
    class IoServicesImpl;

    class IoServices final {
        std::unique_ptr<IoServicesImpl> _impl;
        
    public:
        IoServices();
        ~IoServices();

		void Start();

        void AddTcpServer(int port, ReadStreamCallback&& readStream);

		void StartTcpServer(int port);

		void AsyncConnect(ConnectStreamCallback&& connectCb, ReadStreamCallback&& readCb, StreamErrorCallback&& errCb, std::string ipAddress, int port);
        
		std::shared_ptr<DgramListener> BindDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, const std::string& ipAddress, int port);

		void SendToAllServerConnections(const std::string& msg, bool nullTerminate);
        
        void RunWork(std::function<void(void)>const& work);
        void SetPeriodicTimer(
                              PeriodicTimer id,
                              std::chrono::duration<long long>  du,
                              const std::function<void(PeriodicTimer id)>& handler);

    };
    
}

#endif
