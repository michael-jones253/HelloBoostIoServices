//
//  UnixServer.h
//  AsyncIo
//
//  Created by Michael Jones on 01/11/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__UnixServer__
#define __HelloAsio__UnixServer__

#include "UnixStreamConnection.h"
#include "TcpDomainConnection.h"
#include "TcpDomainConnection.h"

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <mutex>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo
{
	/// <summary>
	/// For async IO with client connections.
	/// NB this class is managed internally by the IO services and not to be instantiated by application code.
	/// </summary>
    class UnixServer final
	{
    private:
		boost::asio::io_service* _ioService{};
		std::mutex _mutex{};
		std::string _path{};
		std::unique_ptr<boost::asio::local::stream_protocol::acceptor> _acceptor{};
		std::vector<std::shared_ptr<TcpDomainConnection>> _domainConnections{};
		UnixAcceptStreamCallback _acceptStreamCb{};
		UnixReadStreamCallback _readSomeCb{};
    public:
		UnixServer() = delete;

		/// <summary>
		/// For async IO with client connections.
		/// NB this class is managed internally by the IO services and not to be instantiated by application code.
		/// </summary>
		/// <param name="ioService">The boost IO service.</param>
		/// <param name="path">The path to listen on.</param>
		/// <param name="acceptsStream">Client connection accepted callback.</param>
		/// <param name="readSomeCb">Client read some data callback.</param>
		UnixServer(boost::asio::io_service* ioService, const std::string& path, UnixAcceptStreamCallback&& acceptsStream, UnixReadStreamCallback&& readSomeCb);
        UnixServer(UnixServer&& rhs);
		UnixServer& operator=(UnixServer&& rhs);
		UnixServer(const UnixServer&) = delete;
		UnixServer& operator=(const UnixServer&) = delete;

		~UnixServer();
        void Start();
        void Stop();
        void SendMessageToAllClients(const std::string& msg, bool nullTerminate);
        std::string GetPath() const { return _path; }
    private:
        void OnAccept(std::shared_ptr<TcpDomainConnection> acceptedConn);
        void AcceptHandler(std::shared_ptr<TcpDomainConnection> acceptedConn, const boost::system::error_code& ec);
        void AsyncAccept();
        void ErrorHandler(std::shared_ptr<TcpDomainConnection> conn, boost::system::error_code ec);
        void CloseAllUnixConnections();
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__UnixServer__) */
