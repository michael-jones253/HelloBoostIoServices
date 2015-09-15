//
//  DgramListener.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"

#include "DgramListener.h"
#include "UdpListener.h"
#include <sstream>

using namespace std;

namespace HelloAsio {

    DgramListener::DgramListener(std::shared_ptr<UdpListener> udpListener) :
        _udpListener{udpListener}
    {
        
    }
    
	void DgramListener::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("Connection expired.");
		}

		listener->AsyncWrite(std::move(msg), nullTerminate);
	}

    void DgramListener::ConsumeInto(uint8_t* buf, int len) {
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("Connection expired.");
		}

        memcpy(buf, listener->Data(), len);
        listener->Consume(len);
    }
    
    void DgramListener::ExtractTo(std::vector<uint8_t>& dest, int len) {
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("Connection expired.");
		}

		assert(len <= static_cast<int>(listener->Size()) && "Must extract within limit of available.");
		listener->CopyTo(dest, len);
		listener->Consume(len);
    }
    
    const uint8_t* DgramListener::Data() const {
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("Connection expired.");
		}

		return listener->Data();
    }
    
    size_t DgramListener::Size() const {
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("Connection expired.");
		}

		return listener->Size();
    }
    
    void DgramListener::Consume(int len) {
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("Connection expired.");
		}

		listener->Consume(len);
    }

	IoEndPoint DgramListener::GetPeerEndPoint() const {
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("Connection expired.");
		}

		stringstream addressStr;
		addressStr << listener->PeerEndPoint.address();

		IoEndPoint ep{ addressStr.str(), listener->PeerEndPoint.port() };

		return ep;
	}

    
}