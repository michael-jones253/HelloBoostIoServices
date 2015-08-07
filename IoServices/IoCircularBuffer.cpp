//
//  IoCircularBuffer.cpp
//  HelloAsio
//
//  Created by Michael Jones on 2/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoCircularBuffer.h"

namespace HelloAsio {
    const ssize_t ChunkSize = 99;
    
    IoCircularBuffer::IoCircularBuffer(IoAsyncReadSomeInCallback readSome, IoNotifyAvailableCallback notifySome) :
        _buffer{},
        _readSomeIn(readSome),
        _notifyAvailable(notifySome) {
        
    }

    void IoCircularBuffer::ReadSome() {
        // FIX ME - ensure ReadSome is not called more than once.
        auto requiredCapacity = _buffer.size() + ChunkSize;
        
        // FIX ME - set a limit on capacity.
        if (requiredCapacity > _buffer.capacity()) {
            _buffer.set_capacity(requiredCapacity);
        }

        // ATTENTION - this doesn't work with empty buffer - rethink needed.
        auto nextSlotPtr = &_buffer.back() + 1;
        
        auto handler = std::bind(&IoCircularBuffer::ReadSomeHandler, this, std::placeholders::_1);
        _readSomeIn(nextSlotPtr, ChunkSize, std::move(handler));
    }
    
    void IoCircularBuffer::ReadSomeHandler(ssize_t bytesRead) {
        _buffer.resize(_buffer.size() + bytesRead);
        
        if (bytesRead <= 0) {
            // FIX ME - what to do, throw?
            return;
        }
        
        _notifyAvailable(_buffer.size());
        
        // Chain the next async read.
        ReadSome();
    }
    
    const uint8_t* IoCircularBuffer::Consume(ssize_t len) {
        return _buffer.linearize();
    }

}