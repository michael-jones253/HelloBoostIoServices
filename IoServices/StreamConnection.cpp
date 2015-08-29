//
//  StreamConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "StreamConnection.h"
#include "TcpPeerConnection.h"

namespace HelloAsio {

    StreamConnection::StreamConnection(std::shared_ptr<TcpPeerConnection> peerConnection) :
        _peerConnection{peerConnection}
    {
        
    }
    
    void StreamConnection::ConsumeInto(uint8_t* buf, int len) {
        memcpy(buf, _peerConnection->Data(), len);
        _peerConnection->Consume(len);
    }
    
    const uint8_t* StreamConnection::Data() const {
        return _peerConnection->Data();
    }
    
    ssize_t StreamConnection::Size() const {
        return _peerConnection->Size();
    }
    
    void StreamConnection::Consume(int len) {
        _peerConnection->Consume(len);
    }
    
}