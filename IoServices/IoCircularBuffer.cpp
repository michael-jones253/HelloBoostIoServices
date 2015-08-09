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
            auto rs = _buffer.size();
            /*
            _buffer.resize(requiredCapacity);
            _buffer.resize(rs);
             */
        }

        auto nextSlotPtr = _buffer.data() + _buffer.size();
        assert(nextSlotPtr != nullptr);
        cout << "Buf next slot: " << reinterpret_cast<long long>(nextSlotPtr) << " size: " << _buffer.size() << endl;
        
        auto handler = std::bind(&IoCircularBuffer::ReadSomeHandler, this, std::placeholders::_1);
        _readSomeIn(nextSlotPtr, _chunkSize, std::move(handler));
    }
    
    void IoCircularBuffer::ReadSomeHandler(ssize_t bytesRead) {
        assert(_buffer.size() + bytesRead <= _buffer.capacity());
        array<uint8_t, 1024> x{}; // FIX ME TEMP
        memset(x.data(), 0xFF, 1024);
        assert(memcmp(_buffer.data(), x.data(), bytesRead) == 0);
        cout << "Before resize of " << (_buffer.size() + bytesRead) << endl;
        _buffer.resize(_buffer.size() + bytesRead);
        assert(memcmp(_buffer.data(), x.data(), bytesRead) == 0);
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


}