//
//  IoCircularBuffer.h
//  HelloAsio
//
//  Created by Michael Jones on 2/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__IoCircularBuffer__
#define __HelloAsio__IoCircularBuffer__

#include <vector>
#include <functional>
#include <iostream>

/* The classes below are exported */
#pragma GCC visibility push(default)

template <class T>
class ResizeReserveAllocator
: public std::allocator<T>
{
public:
    using std::allocator<T>::allocator;
    
    // At least on OS X this does not get called by any vector re-allocation methods.
    template <class U> void construct(U* me, const T&rhs) {
        std::cout << "copy construct" << std::endl;
        *me = rhs;
    }

    // This is needed for a reserve which shuffles data from one part of the container to another and requires
    // copying of this data.
    template<class U>
    void construct(U* me, T&& rhs) {
        std::cout << "move construct" << std::endl;
        *me = std::move(rhs);
    };
    
    // This is needed for a resize without default initialisation wiping out data in the reserved/spare capacity area.
    template <class U, class... Args> void construct(U*, Args&&...) {
        std::cout << "variadic construct" << std::endl;
    }
};

namespace HelloAsio {

    using IoAsyncReadSomeInCallback = std::function<void(uint8_t* bufPtr, ssize_t len, std::function<void(ssize_t)>&&handler)>;
    
    using IoNotifyAvailableCallback = std::function<void(ssize_t available)>;

    class IoCircularBuffer {
    private:
        std::vector<uint8_t, ResizeReserveAllocator<uint8_t>> _buffer;
        int _chunkSize;
        IoAsyncReadSomeInCallback _readSomeIn;
        IoNotifyAvailableCallback _notifyAvailable;
    public:
        IoCircularBuffer(IoAsyncReadSomeInCallback readSome);
        IoCircularBuffer();
        
        IoCircularBuffer& operator=(IoCircularBuffer&& rhs);
        
        void BeginReadSome(IoNotifyAvailableCallback&& notifySome, int chunkSize);
        
        void Consume(ssize_t);
        
        uint8_t const* Get() const;
        
        ssize_t Size() const;
        
        // This is only needed for memory consumption diagnostics.
        ssize_t Capacity() const;
        
    private:
        void ReadSome();
        void ReadSomeHandler(ssize_t bytesRead);
    };
}

#pragma GCC visibility pop
#endif /* defined(__HelloAsio__IoCircularBuffer__) */
