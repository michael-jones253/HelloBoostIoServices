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

namespace HelloAsio {
    struct IoBufferWrapper {
        std::string Buffer;
        
        IoBufferWrapper(std::string&& msg);
    };
}

#endif /* defined(__HelloAsio__IoBufferWrapper__) */
