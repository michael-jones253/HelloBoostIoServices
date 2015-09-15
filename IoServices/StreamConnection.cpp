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
    
	void StreamConnection::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		conn->AsyncWrite(std::move(msg), nullTerminate);
	}

    void StreamConnection::ConsumeInto(uint8_t* buf, int len) {
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

        memcpy(buf, conn->Data(), len);
        conn->Consume(len);
    }
    
    void StreamConnection::ExtractTo(std::vector<uint8_t>& dest, int len) {
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		assert(len <= static_cast<int>(conn->Size()) && "Must extract within limit of available.");
		conn->CopyTo(dest, len);
		conn->Consume(len);
    }
    
    const uint8_t* StreamConnection::Data() const {
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		return conn->Data();
    }
    
    size_t StreamConnection::Size() const {
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		return conn->Size();
    }
    
    void StreamConnection::Consume(int len) {
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		conn->Consume(len);
    }

	EndPoint StreamConnection::GetPeerEndPoint() const {
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		stringstream addressStr;
		addressStr << conn->PeerEndPoint.address();

		EndPoint ep{ addressStr.str(), conn->PeerEndPoint.port() };

		return ep;
	}

    
}