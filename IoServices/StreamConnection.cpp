//
//  StreamConnection.cpp
//  AsyncIo
//
//  Created by Michael Jones on 29/08/2015.
//  Copyright (c)All rights reserved.
//
#include "stdafx.h"

#include "StreamConnection.h"
#include "TcpPeerConnection.h"
#include <sstream>

using namespace std;

namespace AsyncIo
{
    StreamConnection::StreamConnection(std::shared_ptr<TcpPeerConnection> peerConnection, bool isClientSide) :
        _peerConnection{peerConnection},
		_isClientSide{ isClientSide }
    {
        
    }

	/// <summary>
	/// For closing the socket and removing the underlying peer connection.
	/// </summary>
	StreamConnection::~StreamConnection()
	{
		if (!_isClientSide)
		{
			// Server manages lifetime of underlying peer connection.
			return;
		}

		// Only client side initiated connections destruct the underlying peer connection.
		auto peerConn = _peerConnection.lock();
		if (peerConn)
		{
			peerConn->PeerSocket.close();
		}
	}

    
	/// <summary>
	/// Asynchronous write of string message.
	/// NB Safe to call in rapid succession. Messages internally queued.
	/// </summary>
	/// <param name="msg">The string message to send.</param>
	/// <param name="nullTerminate">Whether to null terminate the message or not.</param>
	void StreamConnection::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		conn->AsyncWrite(std::move(msg), nullTerminate);
	}

    /// <summary>
    /// Consume from the start of the circular buffer storage and copy to the destination buffer.
    /// </summary>
    /// <param name="buf">The buffer to receive the data.</param>
    /// <param name="len">The amount to be consumed/copied.</param>
	/// <summary>
	/// Consume from the start of the circular buffer storage and copy to the destination buffer.
	/// </summary>
	/// <param name="buf">The buffer to receive the data.</param>
	/// <param name="len">The amount to be consumed/copied.</param>
	void StreamConnection::ConsumeInto(uint8_t* buf, int len)
	{
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

        memcpy(buf, conn->Data(), len);
        conn->Consume(len);
    }
    
	/// <summary>
	/// Consume from the start of the circular buffer storage and copy to the back of the destination.
	/// </summary>
	/// <param name="dest">The destination to receive the data.</param>
	/// <param name="len">The amount to be consumed/copied.</param>
	void StreamConnection::ConsumeTo(std::vector<uint8_t>& dest, int len)
	{
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		assert(len <= static_cast<int>(conn->Size()) && "Must extract within limit of available.");
		conn->CopyTo(dest, len);
		conn->Consume(len);
    }
    
    /// <summary>
    /// Returns a pointer to the available data in the circular buffer.
    /// NB to be used in conjunction with Size.
    /// </summary>
    /// <returns>A pointer to available received data.</returns>
	const uint8_t* StreamConnection::Data() const
	{
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		return conn->Data();
    }
    
	/// <summary>
	/// Returns the size of the available data in the circular buffer.
	/// </summary>
	/// <returns>A pointer to available received data.</returns>
    size_t StreamConnection::Size() const
	{
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		return conn->Size();
    }
    
	/// <summary>
	/// Consume data from the start of the circular buffer.
	/// NB must be called from the context of the callback.
	/// </summary>
	/// <param name="len">The length to be consumed.</param>
	void StreamConnection::Consume(int len)
	{
		auto conn = _peerConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		conn->Consume(len);
    }

	/// <summary>
	/// Gets the peer end point for identification.
	/// </summary>
	/// <returns>The ip address/port information.</returns>
	EndPoint StreamConnection::GetPeerEndPoint() const
	{
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