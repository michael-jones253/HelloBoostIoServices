//
//  IoBufferWrapper.h
//  HelloAsio
//
//  Created by Michael Jones on 31/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__IoBufferWrapper__
#define __HelloAsio__IoBufferWrapper__

#include <string>
#include <vector>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace HelloAsio {
    struct IoBufferWrapper {
        std::vector<uint8_t> Buffer;
        
        IoBufferWrapper(const std::string& msg);
        IoBufferWrapper(std::vector<uint8_t>&& rhs);
        
        IoBufferWrapper(IoBufferWrapper&& rhs);
        IoBufferWrapper& operator=(HelloAsio::IoBufferWrapper&& rhs);

    };
}

#pragma GCC visibility pop
#endif /* defined(__HelloAsio__IoBufferWrapper__) */
