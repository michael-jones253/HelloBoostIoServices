//
//  StreamConnection.h
//  HelloAsio
//
//  Created by Michael Jones on 29/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__StreamConnection__
#define __HelloAsio__StreamConnection__

#include <iostream>
#include <memory>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace HelloAsio {
    // Hide Tcp peer connection.
    class TcpPeerConnection;
    class StreamConnection;
    
    using StreamErrorCallback = std::function<void(std::shared_ptr<StreamConnection>, std::string msg)>;
    
    using ReadStreamCallback = std::function<void(std::shared_ptr<StreamConnection>, int bytesAvailable)>;
    
    class StreamConnection : public std::enable_shared_from_this<StreamConnection> {
    private:
        std::shared_ptr<TcpPeerConnection> _peerConnection;
        
    public:
        StreamConnection(std::shared_ptr<TcpPeerConnection> peerConnection);
        
        void ConsumeInto(uint8_t* buf, int len);
        
        const uint8_t* Data() const;
        
        ssize_t Size() const;
        
        void Consume(int len);
    };
}

#pragma GCC visibility pop
#endif /* defined(__HelloAsio__StreamConnection__) */
