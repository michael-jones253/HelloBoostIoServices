//
//  IoCircularBuffer.cpp
//  AsyncIo
//
//  Created by Michael Jones on 2/08/2015.
//   https://github.com/michael-jones253/HelloBoostIoServices
//
#include "stdafx.h"

#include "IoCircularBuffer.h"
#include <sstream>
#include <iostream>
#include <assert.h>

#include <array>

using namespace std;

namespace
{
	struct ContextGuard
	{
		std::atomic<bool> &HasContext;

		ContextGuard() = delete;

		ContextGuard(std::atomic<bool>& hasContext) : HasContext{hasContext}
		{
			HasContext = true;
		}

		~ContextGuard()
		{
			HasContext = false;
		}
	};
}

namespace AsyncIo {
    
    IoCircularBuffer::IoCircularBuffer(IoAsyncReadSomeInCallback readSome) :
		_hasContext{},
		_shouldRead{},
        _buffer{},
        _chunkSize{},
        _readSomeIn(readSome),
        _notifyAvailable{} {
        
    }
    
    IoCircularBuffer::IoCircularBuffer() :
		_hasContext{},
	_shouldRead{},
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


    void IoCircularBuffer::BeginChainedRead(IoNotifyAvailableCallback&& notifySome, int chunkSize, bool immediate) {
        // Only one chained readsome allowed in progress.
        if (_notifyAvailable != nullptr) {
            throw runtime_error("IO circular buffer - read some already in progress");
        }
        
        _notifyAvailable = move(notifySome);
        _chunkSize = chunkSize;
		_shouldRead = immediate;

		if (immediate) {
			ReadSome();
		}
    }
    
    void IoCircularBuffer::ReadSome() {
        auto requiredCapacity = _buffer.size() + _chunkSize;
        
		if (requiredCapacity >= _buffer.max_size())
		{
			throw runtime_error("Exceeded circular buffer capacity");
		}

        if (requiredCapacity > _buffer.capacity()) {
            // cout << "Before reserve of " << requiredCapacity << endl;
            _buffer.reserve(requiredCapacity);
        }

        auto nextSlotPtr = _buffer.data() + _buffer.size();
        assert(nextSlotPtr != nullptr);
        // cout << "Buf next slot: " << reinterpret_cast<long long>(nextSlotPtr) << " size: " << _buffer.size() << endl;
        
        auto handler = std::bind(&IoCircularBuffer::ReadSomeHandler, this, std::placeholders::_1);

		// NB this is async, so the lock won't be held for long.
        _readSomeIn(nextSlotPtr, _chunkSize, std::move(handler));
    }
    
    void IoCircularBuffer::ReadSomeHandler(size_t bytesRead) {
		size_t available{};
		{
			// Ensure that the application notify available handler only calls _buffer modification/accessors
			// from the context of this handler. This is the only safe place it can do so without
			// thread contention. This handler is called once ReadSome has completed therefore we can guarantee
			// that no buffer operations are happening during the context of this block.
			ContextGuard contextGuard(_hasContext);
			assert(_buffer.size() + bytesRead <= _buffer.capacity());
			// cout << "Before resize of " << (_buffer.size() + bytesRead) << endl;
			_buffer.resize(_buffer.size() + bytesRead);
        
			if (bytesRead == 0) {
				// FIX ME - what to do, throw? This can be called from boost IO service, so maybe not.
				return;
			}

			available = _buffer.size();
			_notifyAvailable(available);
		}
        
		if (_shouldRead) {
			// Chain the next async read.
			ReadSome();
		}
	}
    
    void IoCircularBuffer::Consume(size_t len) {
		assert(_hasContext.load() && "Circular buffer must have the callback context to consume");
		if (!_hasContext.load())
		{
			throw runtime_error("Circular buffer must have the callback context to consume");
		}

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
    
    void IoCircularBuffer::CopyTo(std::vector<uint8_t>& dest, int len) {
		assert(_hasContext.load() && "Circular buffer must have the callback context to consume");
		if (!_hasContext.load())
		{
			throw runtime_error("Circular buffer must have the callback context to consume");
		}

		std::copy(_buffer.begin(), _buffer.begin() + len, std::back_inserter(dest));
    }

    
    uint8_t const* IoCircularBuffer::Get() const {
		assert(_hasContext.load() && "Circular buffer must have the callback context to get data pointer");
		if (!_hasContext.load())
		{
			throw runtime_error("Circular buffer must have the callback context to get data pointer");
		}

        return _buffer.data();
    }

    size_t IoCircularBuffer::Size() const {
		assert(_hasContext.load() && "Circular buffer must have the callback context to get size");
		if (!_hasContext.load())
		{
			throw runtime_error("Circular buffer must have the callback context to get size");
		}

        return _buffer.size();
    }
    
    size_t IoCircularBuffer::Capacity() const {
		assert(_hasContext.load() && "Circular buffer must have the callback context to get capacity");
		if (!_hasContext.load())
		{
			throw runtime_error("Circular buffer must have the callback context to get capacity");
		}

        return _buffer.capacity();
    }

}