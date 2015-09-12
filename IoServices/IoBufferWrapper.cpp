//
//  IoBufferWrapper.cpp
//  HelloAsio
//
//  Created by Michael Jones on 31/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"
#include "IoBufferWrapper.h"
using namespace std;

namespace HelloAsio {
    IoBufferWrapper::IoBufferWrapper(const std::string& msg) {
        Buffer.insert(Buffer.begin(), msg.cbegin(), msg.cend());

		// Seem to need this on windows.
		Buffer.push_back('\0');
    }
    
    IoBufferWrapper::IoBufferWrapper(vector<uint8_t>&& rhs) {
        Buffer = move(rhs);
    }
    
    IoBufferWrapper::IoBufferWrapper(IoBufferWrapper&& rhs) {
        Buffer = move(rhs.Buffer);
    }
    
    IoBufferWrapper& IoBufferWrapper::operator=(IoBufferWrapper&& rhs) {
        Buffer = move(rhs.Buffer);
        return *this;
    }

}
