//
//  UnixConnectionManager.h
//  AsyncIo
//
//  Created by Michael Jones on 08/11/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__UnixConnectionManager__
#define __HelloAsio__UnixConnectionManager__

#include <IoServices/IoServices.h>
#include <memory>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace UnixClient
{
    class UnixConnectionManagerImpl;

	/// <summary>
	/// For async IO with client connections.
	/// NB this class is managed internally by the IO services and not to be instantiated by application code.
	/// </summary>
    class UnixConnectionManager final
	{
    private:
		std::unique_ptr<UnixConnectionManagerImpl> _impl;

    public:
		UnixConnectionManager() = delete;

		/// <summary>
		/// For async IO with client connections.
		/// NB this class is managed internally by the IO services and not to be instantiated by application code.
		/// </summary>
		/// <param name="ioService">The boost IO service.</param>
		/// <param name="path">The path to listen on.</param>
		/// <param name="acceptsStream">Client connection accepted callback.</param>
		/// <param name="readSomeCb">Client read some data callback.</param>
		UnixConnectionManager(AsyncIo::IoServices& ioService, std::chrono::seconds timeout);

        UnixConnectionManager(UnixConnectionManager&& rhs);
		UnixConnectionManager& operator=(UnixConnectionManager&& rhs);
		UnixConnectionManager(const UnixConnectionManager&) = delete;
		UnixConnectionManager& operator=(const UnixConnectionManager&) = delete;

		~UnixConnectionManager();
        void Start();
        void Stop();

        bool TryAddConnection(const std::string& path);
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__UnixConnectionManager__) */
