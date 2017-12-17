//
//  UnixStreamConnection.h
//  AsyncIo
//
//  Created by Michael Jones on 29/08/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__UnixStreamConnection__
#define __HelloAsio__UnixStreamConnection__

#include <iostream>
#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo
{
    // Hide Tcp unix connection.
    class TcpDomainConnection;
    class UnixStreamConnection;
    
	using UnixStreamConnectionErrorCallback = std::function<void(const std::string& msg)>;

	using UnixStreamIoErrorCallback = std::function<void(std::shared_ptr<UnixStreamConnection>, const std::string& msg)>;
    
    using UnixReadStreamCallback = std::function<void(std::shared_ptr<UnixStreamConnection>, int bytesAvailable)>;

	using UnixAcceptStreamCallback = std::function<void(std::shared_ptr<UnixStreamConnection> acceptedConn)>;

	using UnixConnectStreamCallback = std::function<void(std::shared_ptr<UnixStreamConnection>)>;

	struct UnixEndPoint
	{
		std::string DomainAddress;
	};

	/// <summary>
	/// For logging and error reporting.
	/// </summary>
	/// <param name="os">Output stream.</param>
	/// <param name="groupType">The end point.</param>
	/// <returns>End point as readable text stream.</returns>
	inline std::ostream& operator<<(std::ostream &os, const UnixEndPoint &endPoint)
	{
		os << endPoint.DomainAddress;

		return os;
	}

	/// <summary>
	/// For logging and error reporting.
	/// </summary>
	/// <param name="os">Output stream.</param>
	/// <param name="groupType">The end point.</param>
	/// <returns>End point as readable text stream.</returns>
	inline std::wostream& operator<<(std::wostream &os, const UnixEndPoint &endPoint)
	{
		os << std::wstring(endPoint.DomainAddress.begin(), endPoint.DomainAddress.end());

		return os;
	}

    /// <summary>
    /// Wrapper class to hide the boost implementation of the TCP Unix connection.
    /// NB this class is generated by IO services method and not to be instantiated by application code.
    /// </summary>
    class UnixStreamConnection : public std::enable_shared_from_this<UnixStreamConnection> {
    private:
        std::weak_ptr<TcpDomainConnection> _domainConnection;

		bool _isClientSide;
        
    public:
		// Review - make private and a friend of IO services.
        UnixStreamConnection(std::shared_ptr<TcpDomainConnection> domainConnection, bool isClientSide);

		/// <summary>
		/// For closing the socket and removing the underlying domain connection.
		/// </summary>
		~UnixStreamConnection();

		/// <summary>
		/// Returns whether the connection has expired (been closed).
		/// </summary>
		/// <returns>True if expired.</returns>
		bool Expired() const;

		/// <summary>
		/// Asynchronous write of string message.
		/// NB Safe to call in rapid succession. Messages internally queued.
		/// </summary>
		/// <param name="msg">The string message to send.</param>
		/// <param name="nullTerminate">Whether to null terminate the message or not.</param>
		/// <returns>The backlog of messages queued for writing.</returns>
		int AsyncWrite(std::string&& msg, bool nullTerminate);

		/// <summary>
		/// Asynchronous write of byte vector message.
		/// </summary>
		/// <param name="msg">The message to send.</param>
		/// <returns>The backlog of messages queued for writing.</returns>
		int AsyncWrite(std::vector<uint8_t>&& msg);

		/// <summary>
		/// Consume from the start of the circular buffer storage and copy to the destination buffer.
		/// </summary>
		/// <param name="buf">The buffer to receive the data.</param>
		/// <param name="len">The amount to be consumed/copied.</param>
		void ConsumeInto(uint8_t* buf, int len);
        
		/// <summary>
		/// Consume from the start of the circular buffer storage and copy to the back of the destination.
		/// </summary>
		/// <param name="dest">The destination to receive the data.</param>
		/// <param name="len">The amount to be consumed/copied.</param>
        void ConsumeTo(std::vector<uint8_t>& dest, int len);
        
		/// <summary>
		/// Returns a pointer to the available data in the circular buffer.
		/// NB to be used in conjunction with Size.
		/// </summary>
		/// <returns>A pointer to available received data.</returns>
		const uint8_t* Data() const;
        
		/// <summary>
		/// Returns the size of the available data in the circular buffer.
		/// </summary>
		/// <returns>A pointer to available received data.</returns>
		size_t Size() const;
        
		/// <summary>
		/// Consume data from the start of the circular buffer.
		/// NB must be called from the context of the callback.
		/// </summary>
		/// <param name="len">The length to be consumed.</param>
        void Consume(int len);

		/// <summary>
		/// Gets the domain end point for identification.
		/// </summary>
		/// <returns>The ip address/port information.</returns>
		UnixEndPoint GetDomainEndPoint() const;

		/// <summary>
		/// Close the underlying socket.
		/// </summary>
		void Close();
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__UnixStreamConnection__) */