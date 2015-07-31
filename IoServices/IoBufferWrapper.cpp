//
//  IoBufferWrapper.cpp
//  HelloAsio
//
//  Created by Michael Jones on 31/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoBufferWrapper.h"

namespace HelloAsio {
    IoBufferWrapper::IoBufferWrapper(std::string&& msg) {
        Buffer = std::move(msg);
    }

}
