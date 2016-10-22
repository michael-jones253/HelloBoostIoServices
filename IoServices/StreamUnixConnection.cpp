//
//  StreamUnixConnection.cpp
//  AsyncIo
//
//  Created by Michael Jones on 29/08/2015.
//   https://github.com/michael-jones253/HelloBoostIoServices
//
#include "stdafx.h"

#include "StreamUnixConnection.h"
#include "TcpDomainConnection.h"
#include <sstream>

using namespace std;

namespace AsyncIo
{
    StreamUnixConnection::StreamUnixConnection(std::shared_ptr<TcpDomainConnection> domainConnection, bool isClientSide) :
        _domainConnection{domainConnection},
		_isClientSide{ isClientSide }
    {
        
    }

	/// <summary>
	/// For closing the socket and removing the underlying domain connection.
	/// </summary>
	StreamUnixConnection::~StreamUnixConnection()
	{
		auto domainConn = _domainConnection.lock();
		if (domainConn)
		{
			domainConn->DomainSocket.close();
		}
	}
    
	/// <summary>
	/// Returns whether the connection has expired (been closed).
	/// </summary>
	/// <returns>True if expired.</returns>
	bool StreamUnixConnection::Expired() const
	{
		return _domainConnection.expired();
	}

	/// <summary>
	/// Asynchronous write of string message.
	/// NB Safe to call in rapid succession. Messages internally queued.
	/// </summary>
	/// <param name="msg">The string message to send.</param>
	/// <param name="nullTerminate">Whether to null terminate the message or not.</param>
	/// <returns>The backlog of messages queued for writing.</returns>
	int StreamUnixConnection::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto conn = _domainConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		return conn->AsyncWrite(std::move(msg), nullTerminate);
	}

	/// <summary>
	/// Asynchronous write of byte vector message.
	/// </summary>
	/// <param name="msg">The message to send.</param>
	/// <returns>The backlog of messages queued for writing.</returns>
	int StreamUnixConnection::AsyncWrite(std::vector<uint8_t>&& msg)
	{
		auto conn = _domainConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		return conn->AsyncWrite(std::move(msg));
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
	void StreamUnixConnection::ConsumeInto(uint8_t* buf, int len)
	{
		auto conn = _domainConnection.lock();
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
	void StreamUnixConnection::ConsumeTo(std::vector<uint8_t>& dest, int len)
	{
		auto conn = _domainConnection.lock();
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
	const uint8_t* StreamUnixConnection::Data() const
	{
		auto conn = _domainConnection.lock();
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
    size_t StreamUnixConnection::Size() const
	{
		auto conn = _domainConnection.lock();
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
	void StreamUnixConnection::Consume(int len)
	{
		auto conn = _domainConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		conn->Consume(len);
    }

	/// <summary>
	/// Gets the domain end point for identification.
	/// </summary>
	/// <returns>The ip address/port information.</returns>
	UnixEndPoint StreamUnixConnection::GetDomainEndPoint() const
	{
		auto conn = _domainConnection.lock();
		if (!conn)
		{
			throw runtime_error("Connection expired.");
		}

		stringstream addressStr;
		addressStr << conn->DomainEndPoint.path();

		UnixEndPoint ep{ addressStr.str() };

		return ep;
	}

	/// <summary>
	/// Close the underlying socket.
	/// </summary>
	void StreamUnixConnection::Close()
	{
		auto conn = _domainConnection.lock();
		if (!conn)
		{
			// Already closed.
			return;
		}

		conn->Close();
	}
}
