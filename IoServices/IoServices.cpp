#include "stdafx.h"

#include "IoServices.h"
#include "IoServicesImpl.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

namespace
{
	const int ReadSomeChunkSize = 12;
	const int DatagramMtu = 65535;
}

namespace AsyncIo
{
	IoServices::IoServices(IoExceptionCallback&& exceptionCb) : _impl(std::make_unique<IoServicesImpl>(move(exceptionCb)))
	{
	}

	IoServices::~IoServices()
	{

	}

	/// <summary>
	/// Starts the IO services.
	/// NB This does not block.
	/// </summary>
	void IoServices::Start()
	{
		auto started = _impl->Start();

		// Wait for io service to get running, to prevent any races.
		auto timeout = seconds(5);
		auto ok = started.wait_for(timeout);
		if (ok == future_status::timeout)
		{
			throw runtime_error("Io service failed to start");
		}
	}

	/// <summary>
	/// Stops the IO services.
	/// NB This blocks.
	void IoServices::Stop()
	{
		_impl->Stop();
	}

	/// <summary>
	/// Initialise a periodic timer.
	/// </summary>
	/// <param name="id">The unique periodic timer id.</param>
	void IoServices::InitialisePeriodicTimer(const PeriodicTimer& id)
	{
		_impl->InitialisePeriodicTimer(id.Id, id.Name);
	}

	/// <summary>
	/// Initialise a one shot timer timer.
	/// </summary>
	/// <param name="id">The unique timer id.</param>
	/// <param name="name">The name for logging.</param>
	void IoServices::InitialiseOneShotTimer(const Timer& id)
	{
		_impl->InitialiseOneShotTimer(id.Id, id.Name);
	}


	/// <summary>
	/// Add a TCP server to listen on the specified port.
	/// </summary>
	/// <param name="port">The port to listen on.</param>
	/// <param name="acceptsStream">Client connection accepted callback.</param>
	/// <param name="readStream">Client connection data received callback.</param>
	void IoServices::AddTcpServer(int port, AcceptStreamCallback&& acceptsStream, ReadStreamCallback&& readStream)
	{
		// take a copy to bind it to each server's read stream callback.
		_impl->AddTcpServer(port, std::move(acceptsStream), std::move(readStream));
	}

	/// <summary>
	/// Starts the TCP server.
	/// NB all servers must be added first due to move issues.
	/// </summary>
	/// <param name="port">Identifies the server to start.</param>
	void IoServices::StartTcpServer(int port)
	{
		_impl->StartTcpServer(port);
	}

	/// <summary>
	/// NB private Asynchronous connect handler.
	/// </summary>
	/// <param name="connectCb">Connect stream callback passed in by value to get a copy.</param>
	/// <param name="readCb">Read stream callback passed in by value to get a copy.</param>
	/// <param name="errCb">Stream error callback passed in by value to get a copy.</param>
	/// <param name="peerConn">The peer connection generated during the connection.</param>
	void ConnectHandler(ConnectStreamCallback connectCb, ReadStreamCallback readCb, StreamIoErrorCallback errCb, std::shared_ptr<TcpPeerConnection> peerConn)
	{
		auto streamConnection = make_shared<StreamConnection>(peerConn, true /* is client side */);

		// Take a weak pointer to the stream connection and a copy of the callback to bind them to lambda.
		// The lambda then gets moved into the peer connection which ensures that the callback will be
		// associated with this peer connection and last the lifetime of the peer connection.
		// The weak pointer can be used to give a guaranteed handle to the parent stream connection and avoid circular ownership
		// destruction problems.
		auto weakStreamConn = weak_ptr<StreamConnection>(streamConnection);
		auto available = [weakStreamConn, readCb, peerConn](size_t available) {
			auto lockedConn = weakStreamConn.lock();
			if (lockedConn)
			{
				readCb(lockedConn, available);
			}
			else
			{
				// If this happens examine the application code for any destruction order design issues.
				std::cout << "Client side received data with no handler available: " << peerConn->PeerEndPoint << available << std::endl;
			}
		};

		auto errorHandler = [weakStreamConn, errCb](std::shared_ptr<TcpPeerConnection> conn, const boost::system::error_code& ec) {
			stringstream errStr;
			errStr << "Stream connection error: " << ec.message();

			// NB this may be null, caller of connect must test for this in error callback.
			auto lockedConn = weakStreamConn.lock();
			errCb(lockedConn, errStr.str());
		};

		peerConn->BeginChainedRead(std::move(available), std::move(errorHandler), ReadSomeChunkSize);

		connectCb(streamConnection);
	}

	/// <summary>
	///  Asynchronous connect handler. Once connected the stream will begin asynchronous reading straight away.
	/// </summary>
	/// <param name="connectCb">The connected callback.</param>
	/// <param name="readCb">Asynchronous data read callback.</param>
	/// <param name="connErrCb">Error callback if socket connect errors encountered.</param>
	/// <param name="ioErrCb">Error callback if socket I/O errors encountered.</param>
	/// <param name="ipAddress">The destination IP.</param>
	/// <param name="port">The destination port.</param>
	void IoServices::AsyncConnect(ConnectStreamCallback&& connectCb, ReadStreamCallback&& readCb, StreamConnectionErrorCallback&& connErrCb, StreamIoErrorCallback&& ioErrCb, std::string ipAddress, int port)
	{
		auto connect = bind(ConnectHandler, std::move(connectCb), std::move(readCb), std::move(ioErrCb), std::placeholders::_1);

		auto error = [connErrCb, ipAddress, port](std::shared_ptr<TcpPeerConnection> conn, const boost::system::error_code& ec) {
			stringstream errStr;
			errStr << "Client connection error on: " << ipAddress << ":" << port << " " << ec.message();
			connErrCb(errStr.str());
		};

		_impl->AsyncConnect(std::move(connect), std::move(error), ipAddress, port);
	}


	void SetupReadCallbacks(shared_ptr<DgramListener> sharedListener, DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, IoServicesImpl* impl, std::shared_ptr<UdpListener> udpListener, bool immediate = true)
	{

		// NB if we inject a shared pointer to the datagram Listener into the UDP listener callback
		// it would means that the UDP listener effectively owns the parent datagram listener. This
		// failure to destruct problems. So use weak pointers to break circular ownership.
		auto weakListener = weak_ptr<DgramListener>(sharedListener);
		auto notify = [weakListener, receiveCb](size_t available) {
			auto lockedListener = weakListener.lock();
			if (lockedListener)
			{
				// Got a lock onto a pointer to the listener. Therefore parent listener
				// is guaranteed not to destruct while we call back into it.
				receiveCb(lockedListener, available);
			}
		};

		auto udpErrCb = [impl, errCb, weakListener](std::shared_ptr<UdpListener> listener, const boost::system::error_code& ec) {
			auto lockedListener = weakListener.lock();
			if (lockedListener)
			{
				stringstream errStr;
				errStr << "Datagram listener error: " << listener->PeerEndPoint << " " << ec.message() << endl;

				// Call the application error handler first.
				errCb(lockedListener, errStr.str());
			}
			else
			{
				stringstream errStr;
				errStr << "Datagram listener destructed: " << listener->PeerEndPoint << " " << ec.message();
				errCb(nullptr, errStr.str());
			}

			// Then call the cleanup handler.
			impl->ErrorHandler(listener, ec);
		};

		udpListener->SetupChainedRead(move(notify), move(udpErrCb), DatagramMtu, immediate);
	}

	/// <summary>
	/// Binds a datagram listener for asynchronous UDP receive.
	/// NB. This will start receiving straight away.
	/// </summary>
	/// <param name="receiveCb">The receive callback for UDP receive.</param>
	/// <param name="errCb">The error callback for socket errors.</param>
	/// <param name="ipAddress">The IP interface address to bind. Can be 0.0.0.0 for address any.</param>
	/// <param name="port">The port to bind to.</param>
	/// <returns>An instance that is listening for datagrams.</returns>
	shared_ptr<DgramListener> IoServices::BindDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, const std::string& ipAddress, int port)
	{
		auto udpListener = _impl->BindDgramListener(ipAddress, port);
		
		// The binded listener to return.
		auto sharedListener = make_shared<DgramListener>(udpListener);

		SetupReadCallbacks(sharedListener, move(receiveCb), move(errCb), _impl.get(), udpListener);

		return sharedListener;
	}

	shared_ptr<DgramListener> IoServices::BindDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, int port)
	{
		auto udpListener = _impl->BindDgramListener(port);

		// The binded listener to return.
		auto sharedListener = make_shared<DgramListener>(udpListener);

		SetupReadCallbacks(sharedListener, move(receiveCb), move(errCb), _impl.get(), udpListener);

		return sharedListener;
	}

	std::shared_ptr<DgramListener> IoServices::UnboundDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb) {
		auto udpListener = _impl->MakeUnboundUdpListener();

		// The binded listener to return.
		auto sharedListener = make_shared<DgramListener>(udpListener);

		SetupReadCallbacks(sharedListener, move(receiveCb), move(errCb), _impl.get(), udpListener, false /* defer read */); // The async read cannot be started now because the socket is not ready until connected.
		return sharedListener;
	}

	void IoServices::SendToAllServerConnections(const std::string& msg, bool nullTerminate)
	{
		_impl->SendToAllServerConnections(msg, nullTerminate);
	}

	void IoServices::RunWork(std::function<void(void)>const& work)
	{
		_impl->RunWork(work);
	}

	/// <summary>
	/// Sets a periodic timer which calls back on the specified interval.
	/// </summary>
	/// <param name="id">The timer ID.</param>
	/// <param name="du">Duration of interval.</param>
	/// <param name="handler">The handler that is called when the timer expires.</param>
	void IoServices::SetPeriodicTimer(
		PeriodicTimer id,
		std::chrono::duration<long long>  du,
		const std::function<void(PeriodicTimer id)>& handler)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(du);
		auto ptimeFromNow = boost::posix_time::milliseconds(ms.count());
		_impl->SetPeriodicTimer(id, ptimeFromNow, handler);
	}

	/// <summary>
	/// Cancel the periodic timer. Cannot guarantee that we will get there just before it goes off.
	/// So timer applications must protect the callback with a predicate.
	/// </summary>
	/// <param name="id">The timer id.</param>
	void IoServices::CancelPeriodicTimer(PeriodicTimer id)
	{
		_impl->CancelPeriodicTimer(id);
	}

}