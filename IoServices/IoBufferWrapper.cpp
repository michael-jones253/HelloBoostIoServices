//
//  IoBufferWrapper.cpp
//  AsyncIo
//
//  Created by Michael Jones on 31/07/2015.
//  Copyright (c)All rights reserved.
//
#include "stdafx.h"
#include "IoBufferWrapper.h"
using namespace std;

namespace AsyncIo {
	IoBufferWrapper::IoBufferWrapper(const std::string& msg, bool nullTerminate) {
        Buffer.insert(Buffer.begin(), msg.cbegin(), msg.cend());

		if (nullTerminate) {
			Buffer.push_back('\0');
		}
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
