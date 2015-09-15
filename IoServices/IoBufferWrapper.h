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
#include <cstdint>

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace HelloAsio {
    struct IoBufferWrapper {
        std::vector<uint8_t> Buffer;
        
        IoBufferWrapper(const std::string& msg, bool nullTerminate);
        IoBufferWrapper(std::vector<uint8_t>&& rhs);
        
        IoBufferWrapper(IoBufferWrapper&& rhs);
        IoBufferWrapper& operator=(HelloAsio::IoBufferWrapper&& rhs);

    };
}

#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__IoBufferWrapper__) */
