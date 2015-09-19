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
#include "StreamConnection.h"
#include "DgramListener.h"
#include <memory>
#include <functional>
#include <chrono>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo
{
    class IoServicesImpl;

	/// <summary>
	/// Provides an API to boost's asio services (async IO, deadline timer and thread pooling).
	/// </summary>
    class IoServices final
	{
        std::unique_ptr<IoServicesImpl> _impl;
        
    public:
        IoServices();
        ~IoServices();

		/// <summary>
		/// Starts the IO services.
		/// NB This does not block.
		/// </summary>
		void Start();

		/// <summary>
		/// Add a TCP server to listen on the specified port.
		/// </summary>
		/// <param name="port">The port to listen on.</param>
		/// <param name="readStream">Client connection data received callback.</param>
        void AddTcpServer(int port, ReadStreamCallback&& readStream);

		/// <summary>
		/// Starts the TCP server.
		/// NB all servers must be added first due to move issues.
		/// </summary>
		/// <param name="port">Identifies the server to start.</param>
		void StartTcpServer(int port);

		/// <summary>
		///  Asynchronous connect handler. Once connected the stream will begin asynchronous reading straight away.
		/// </summary>
		/// <param name="connectCb">The connected callback.</param>
		/// <param name="readCb">Asynchronous data read callback.</param>
		/// <param name="errCb">Error callback if socket errors encountered.</param>
		/// <param name="ipAddress">The destination IP.</param>
		/// <param name="port">The destination port.</param>
		void AsyncConnect(ConnectStreamCallback&& connectCb, ReadStreamCallback&& readCb, StreamErrorCallback&& errCb, std::string ipAddress, int port);
        
		/// <summary>
		/// Binds a datagram listener for asynchronous UDP receive.
		/// NB. This will start receiving straight away.
		/// </summary>
		/// <param name="receiveCb">The receive callback for UDP receive.</param>
		/// <param name="errCb">The error callback for socket errors.</param>
		/// <param name="ipAddress">The IP address to bind.</param>
		/// <param name="port">The port to bind to.</param>
		/// <returns>An instance that is listening for datagrams.</returns>
		std::shared_ptr<DgramListener> BindDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, const std::string& ipAddress, int port);

		void SendToAllServerConnections(const std::string& msg, bool nullTerminate);
        
        void RunWork(std::function<void(void)>const& work);

		/// <summary>
		/// Sets a periodic timer which calls back on the specified interval.
		/// </summary>
		/// <param name="id">The timer ID.</param>
		/// <param name="du">Duration of interval.</param>
		/// <param name="handler">The handler that is called when the timer expires.</param>
        void SetPeriodicTimer(
                              PeriodicTimer id,
                              std::chrono::duration<long long>  du,
                              const std::function<void(PeriodicTimer id)>& handler);

    };
    
}

#endif
