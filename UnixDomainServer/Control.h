//
//  Control.h
//  AsyncIo
//
//  Created by Michael Jones on 19/11/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__Control__
#define __HelloAsio__Control__

#include <string>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace UnixServer
{
    using ConnectCallback = std::function<void(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection>)>;

    using ErrorCallback = std::function<void(const std::string& path, std::shared_ptr<AsyncIo::UnixStreamConnection>, const std::string& msg)>;

    class ControlImpl;

	/// <summary>
	/// For async IO with client connections.
	/// NB this class is managed internally by the IO services and not to be instantiated by application code.
	/// </summary>
    class Control final
	{
    private:
		std::unique_ptr<ControlImpl> _impl;

    public:
		Control() = delete;

		/// <summary>
		/// For async IO with client connections.
		/// NB this class is managed internally by the IO services and not to be instantiated by application code.
		/// </summary>
		/// <param name="ioService">The boost IO service.</param>
		/// <param name="path">The path to listen on.</param>
		/// <param name="acceptsStream">Client connection accepted callback.</param>
		/// <param name="readSomeCb">Client read some data callback.</param>
		Control(CommandId);

        Control(Control&& rhs);
		Control& operator=(Control&& rhs);
		Control(const Control&) = delete;
		Control& operator=(const Control&) = delete;

		~Control();
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

#endif /* defined(__HelloAsio__Control__) */
