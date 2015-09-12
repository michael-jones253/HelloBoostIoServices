//
//  StreamConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"

#include "StreamConnection.h"
#include "TcpPeerConnection.h"
#include <sstream>

using namespace std;

namespace HelloAsio {

    StreamConnection::StreamConnection(std::shared_ptr<TcpPeerConnection> peerConnection) :
        _peerConnection{peerConnection}
    {
        
    }
    
    void StreamConnection::ConsumeInto(uint8_t* buf, int len) {
        memcpy(buf, _peerConnection->Data(), len);
        _peerConnection->Consume(len);
    }
    
    void StreamConnection::ExtractTo(std::vector<uint8_t>& dest, int len) {
        assert(len <= static_cast<int>(_peerConnection->Size()) && "Must extract within limit of available.");
        _peerConnection->CopyTo(dest, len);
        _peerConnection->Consume(len);
    }
    
    const uint8_t* StreamConnection::Data() const {
        return _peerConnection->Data();
    }
    
    size_t StreamConnection::Size() const {
        return _peerConnection->Size();
    }
    
    void StreamConnection::Consume(int len) {
        _peerConnection->Consume(len);
    }

	EndPoint StreamConnection::GetPeerEndPoint() const {
		stringstream addressStr;
		addressStr << _peerConnection->PeerEndPoint.address();

		EndPoint ep{addressStr.str(), _peerConnection->PeerEndPoint.port()};

		return ep;
	}

    
}