//
//  IoCircularBuffer.h
//  HelloAsio
//
//  Created by Michael Jones on 2/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__IoCircularBuffer__
#define __HelloAsio__IoCircularBuffer__

#include <boost/circular_buffer.hpp>
#include <functional>

/* The classes below are exported */
#pragma GCC visibility push(default)


namespace HelloAsio {

    using IoAsyncReadSomeInCallback = std::function<void(uint8_t* bufPtr, ssize_t len, std::function<void(ssize_t)>&&handler)>;
    
    using IoNotifyAvailableCallback = std::function<void(ssize_t available)>;

    class IoCircularBuffer {
    private:
        boost::circular_buffer<uint8_t> _buffer;
        IoAsyncReadSomeInCallback _readSomeIn;
        IoNotifyAvailableCallback _notifyAvailable;
    public:
        IoCircularBuffer(IoAsyncReadSomeInCallback readSome, IoNotifyAvailableCallback notifySome);
        
        void ReadSome();
        
        const uint8_t* Consume(ssize_t);
    private:
        void ReadSomeHandler(ssize_t bytesRead);
    };
}

#pragma GCC visibility pop
#endif /* defined(__HelloAsio__IoCircularBuffer__) */
