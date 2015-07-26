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

#include <memory>
#include <functional>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace HelloAsio {

    class IoServicesImpl;
    
    class IoServices final {
        std::unique_ptr<IoServicesImpl> _impl;
    public:
        IoServices();
        ~IoServices();
        
        void RunWork(std::function<void(void)>const& work);
    };
    
}

#pragma GCC visibility pop
#endif
