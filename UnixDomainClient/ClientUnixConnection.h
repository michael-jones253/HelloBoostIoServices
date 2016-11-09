//
//  ClientUnixConnection.h
//  AsyncIo
//
//  Created by Michael Jones on 08/11/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__ClientUnixConnection__
#define __HelloAsio__ClientUnixConnection__

#include "UnixStreamConnection.h"
#include <memory>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

#include <IoServices/IoServices.h>

namespace UnixClient
{
    using ConnectCallback = std::function<void(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection>)>;

    using ErrorCallback = std::function<void(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection>, const std::string& msg)>;

    class ClientUnixConnectionImpl;

	/// <summary>
	/// For async IO with client connections.
	/// NB this class is managed internally by the IO services and not to be instantiated by application code.
	/// </summary>
    class ClientUnixConnection final
	{
    private:
		std::unique_ptr<ClientUnixConnectionImpl> _impl;

    public:
		ClientUnixConnection() = delete;

		/// <summary>
		/// For async IO with client connections.
		/// NB this class is managed internally by the IO services and not to be instantiated by application code.
		/// </summary>
		/// <param name="ioService">The boost IO service.</param>
		/// <param name="path">The path to listen on.</param>
		/// <param name="acceptsStream">Client connection accepted callback.</param>
		/// <param name="readSomeCb">Client read some data callback.</param>
		ClientUnixConnection(AsyncIo::IoServices& ioService, const std::string& path, std::chrono::seconds timeout, ConnectCallback&&, ErrorCallback&&);

        ClientUnixConnection(ClientUnixConnection&& rhs);
		ClientUnixConnection& operator=(ClientUnixConnection&& rhs);
		ClientUnixConnection(const ClientUnixConnection&) = delete;
		ClientUnixConnection& operator=(const ClientUnixConnection&) = delete;

		~ClientUnixConnection();
        void Start();
        void Stop();

        void SetConnection(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection> conn);

        void SetError(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection> conn, const std::string& msg);
        bool Expired() const;
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__ClientUnixConnection__) */
