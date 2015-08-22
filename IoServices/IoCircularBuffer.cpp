//
//  IoCircularBuffer.cpp
//  HelloAsio
//
//  Created by Michael Jones on 2/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoCircularBuffer.h"
#include <sstream>
#include <iostream>
#include <assert.h>

#include <array>

using namespace std;



namespace HelloAsio {
    
    IoCircularBuffer::IoCircularBuffer(IoAsyncReadSomeInCallback readSome) :
        _buffer{},
        _chunkSize{},
        _readSomeIn(readSome),
        _notifyAvailable{} {
        
    }
    
    IoCircularBuffer::IoCircularBuffer() :
    _buffer{},
    _chunkSize{},
    _readSomeIn{},
    _notifyAvailable{} {
        
    }
    
    IoCircularBuffer& IoCircularBuffer::operator=(IoCircularBuffer&& rhs) {
        _buffer = move(rhs._buffer);
        _chunkSize = rhs._chunkSize;
        _readSomeIn = move(rhs._readSomeIn);
        _notifyAvailable = move(rhs._notifyAvailable);
        
        return *this;
    }


    void IoCircularBuffer::BeginReadSome(IoNotifyAvailableCallback&& notifySome, int chunkSize) {
        // Only one chained readsome allowed in progress.
        if (_notifyAvailable != nullptr) {
            throw runtime_error("IO circular buffer - read some already in progress");
        }
        
        _notifyAvailable = move(notifySome);
        _chunkSize = chunkSize;
        ReadSome();
    }
    
    void IoCircularBuffer::ReadSome() {
    
        auto requiredCapacity = _buffer.size() + _chunkSize;
        
        // FIX ME - set a limit on capacity.
        if (requiredCapacity > _buffer.capacity()) {
            cout << "Before reserve of " << requiredCapacity << endl;
            _buffer.reserve(requiredCapacity);
        }

        auto nextSlotPtr = _buffer.data() + _buffer.size();
        assert(nextSlotPtr != nullptr);
        cout << "Buf next slot: " << reinterpret_cast<long long>(nextSlotPtr) << " size: " << _buffer.size() << endl;
        
        auto handler = std::bind(&IoCircularBuffer::ReadSomeHandler, this, std::placeholders::_1);
        _readSomeIn(nextSlotPtr, _chunkSize, std::move(handler));
    }
    
    void IoCircularBuffer::ReadSomeHandler(ssize_t bytesRead) {
        assert(_buffer.size() + bytesRead <= _buffer.capacity());
        cout << "Before resize of " << (_buffer.size() + bytesRead) << endl;
        _buffer.resize(_buffer.size() + bytesRead);
        cout << "Buf start: " << reinterpret_cast<long long>(_buffer.data()) << " size: " << _buffer.size() << endl;
        
        if (bytesRead <= 0) {
            // FIX ME - what to do, throw? This can be called from boost IO service, so maybe not.
            return;
        }
        
        _notifyAvailable(_buffer.size());
        
        // Chain the next async read.
        ReadSome();
    }
    
    void IoCircularBuffer::Consume(ssize_t len) {
        if (len > _buffer.size()) {
            stringstream errStr;
            errStr << "IO circular buffer consume of " << len << ", available: " << _buffer.size();
            throw runtime_error(errStr.str());
        }
        
        auto capacity = _buffer.capacity();
        if (len == _buffer.size()) {
            _buffer.resize(0);
            if (_buffer.capacity() < capacity) {
                _buffer.reserve(capacity);
            }
            
            return;
        }
        
        _buffer.erase(_buffer.begin(), _buffer.begin() + len);
        if (_buffer.capacity() < capacity) {
            _buffer.reserve(capacity);
        }
        
    }
    
    uint8_t const* IoCircularBuffer::Get() const {
        return _buffer.data();
    }

    ssize_t IoCircularBuffer::Size() const {
        return _buffer.size();
    }
    
    ssize_t IoCircularBuffer::Capacity() const {
        return _buffer.capacity();
    }


}