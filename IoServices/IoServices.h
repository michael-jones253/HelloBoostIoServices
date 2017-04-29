/*
 *  IoServices.h
 *  IoServices
 *
 *  Created by Michael Jones on 26/07/2015.
 *  https://github.com/michael-jones253/HelloBoostIoServices
 *
 */

#ifndef IoServices_
#define IoServices_

#include "PeriodicTimer.h"
#include "Timer.h"
#include "StreamConnection.h"
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include "UnixStreamConnection.h"
#endif
#include "DgramListener.h"
#include "SecurityOptions.h"
#include <memory>
#include <functional>
#include <chrono>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo
{
	using IoExceptionCallback = std::function<void(const std::string& msg, const std::exception&)>;
	class IoServicesImpl;

	/// <summary>
	/// Provides an API to boost's asio services (async IO, deadline timer and thread pooling).
	/// </summary>
    class IoServices final
	{
	private:
        std::unique_ptr<IoServicesImpl> _impl;
        
    public:
		IoServices(IoExceptionCallback&& exceptionCb);
		~IoServices();

		/// <summary>
		/// Starts the IO services.
		/// NB This does not block.
		/// </summary>
		void Start();

		/// <summary>
		/// Stops the IO services.
		/// NB This blocks.
		void Stop();

		/// <summary>
		/// Initialise a periodic timer.
		/// </summary>
		/// <param name="id">The unique timer id.</param>
		void InitialisePeriodicTimer(const PeriodicTimer& id);

		/// <summary>
		/// Initialise a one shot timer timer.
		/// </summary>
		/// <param name="id">The unique integer id.</param>
		void InitialiseOneShotTimer(const Timer& name);

		/// <summary>
		/// Add a TCP server to listen on the specified port.
		/// </summary>
		/// <param name="port">The port to listen on.</param>
		/// <param name="acceptsStream">Client connection accepted callback.</param>
		/// <param name="readStream">Client connection data received callback.</param>
		void AddTcpServer(int port, AcceptStreamCallback&& acceptsStream, ReadStreamCallback&& readStream);

		/// <summary>
		/// Starts the TCP server.
		/// NB all servers must be added first due to move issues.
		/// </summary>
		/// <param name="port">Identifies the server to start.</param>
		void StartTcpServer(int port);

		/// <summary>
		/// Starts the TCP server to accept SSL/TLS connections.
		/// </summary>
		/// <param name="port">Identifies the server to start.</param>
		/// <param name="security">The SSL/TLS options.</param>
		void StartTcpServer(int port, SecurityOptions&& security);

		/// <summary>
		///  Asynchronous connect handler. Once connected the stream will begin asynchronous reading straight away.
		/// </summary>
		/// <param name="connectCb">The connected callback.</param>
		/// <param name="readCb">Asynchronous data read callback.</param>
		/// <param name="connErrCb">Error callback if socket connect errors encountered.</param>
		/// <param name="ioErrCb">Error callback if socket I/O errors encountered.</param>
		/// <param name="ipAddress">The destination IP.</param>
		/// <param name="port">The destination port.</param>
		void AsyncConnect(ConnectStreamCallback&& connectCb, ReadStreamCallback&& readCb, StreamConnectionErrorCallback&& connErrCb, StreamIoErrorCallback&& ioErrCb, std::string ipAddress, int port);
        
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
		/// <summary>
		///  Asynchronous connect handler. Once connected the stream will begin asynchronous reading straight away.
		/// </summary>
		/// <param name="connectCb">The connected callback.</param>
		/// <param name="readCb">Asynchronous data read callback.</param>
		/// <param name="connErrCb">Error callback if socket connect errors encountered.</param>
		/// <param name="ioErrCb">Error callback if socket I/O errors encountered.</param>
		/// <param name="path">The destination unix domain path.</param>
		void AsyncConnect(UnixConnectStreamCallback&& connectCb, UnixReadStreamCallback&& readCb, UnixStreamConnectionErrorCallback&& connErrCb, UnixStreamIoErrorCallback&& ioErrCb, std::string path);

		/// <summary>
		/// Add a Unix domain server to listen on the specified path.
		/// </summary>
		/// <param name="path">The path to listen on.</param>
		/// <param name="acceptsStream">Client connection accepted callback.</param>
		/// <param name="readStream">Client connection data received callback.</param>
		void AddUnixServer(const std::string& path, UnixAcceptStreamCallback&& acceptsStream, UnixReadStreamCallback&& readStream);

		/// <summary>
		/// Starts the Unix server.
		/// NB all servers must be added first due to move issues.
		/// </summary>
		/// <param name="path">Identifies the server to start.</param>
		void StartUnixServer(const std::string& path);
#endif
		/// <summary>
		/// Binds a datagram listener for asynchronous UDP receive.
		/// NB. This will start receiving straight away.
		/// </summary>
		/// <param name="receiveCb">The receive callback for UDP receive.</param>
		/// <param name="errCb">The error callback for socket errors.</param>
		/// <param name="ipAddress">The IP interface address to bind. Can be 0.0.0.0 for address any.</param>
		/// <param name="port">The port to bind to.</param>
		/// <returns>An instance that is listening for datagrams.</returns>
		std::shared_ptr<DgramListener> BindDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, const std::string& ipAddress, int port);

		// Overload for address any.
		std::shared_ptr<DgramListener> BindDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, int port);

		// For sockets that are not bound to a listening port. Any reads will be on the ephemeral port.
		std::shared_ptr<DgramListener> UnboundDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb);

		void SendToAllServerConnections(const std::string& msg, bool nullTerminate);
        
        void RunWork(std::function<void(void)>const& work);

		/// <summary>
		/// Sets a periodic timer which calls back on the specified interval.
		/// </summary>
		/// <param name="id">The timer ID.</param>
		/// <param name="du">Duration of interval.</param>
		/// <param name="handler">The handler that is called when the timer expires.</param>
        void SetPeriodicTimer(PeriodicTimer id,
                              std::chrono::duration<long long>  du,
                              const std::function<void(PeriodicTimer id)>&& handler);

		/// <summary>
		/// Cancel the periodic timer. Cannot guarantee that we will get there just before it goes off.
		/// So timer applications must protect the callback with a predicate.
		/// </summary>
		/// <param name="id">The timer id.</param>
		void CancelPeriodicTimer(PeriodicTimer id);

    };
    
}

#endif
