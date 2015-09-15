#include "stdafx.h"

#include "IoServices.h"
#include "IoServicesImpl.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

namespace
{
	const int ReadSomeChunkSize = 12;
}

namespace HelloAsio {
	IoServices::IoServices() : _impl(std::make_unique<IoServicesImpl>()) {
	}

	IoServices::~IoServices() {

	}

	void IoServices::Start() {
		auto started = _impl->Start();

		// Wait for io service to get running, to prevent any races.
		auto timeout = seconds(5);
		auto ok = started.wait_for(timeout);
		if (ok == future_status::timeout)
		{
			throw runtime_error("Io service failed to start");
		}
	}

	void IoServices::AddTcpServer(int port, ReadStreamCallback&& readStream) {
		// take a copy to bind it to each server's read stream callback.
		auto serverReadStream = move(readStream);
		auto readSome = [this, serverReadStream](std::shared_ptr<TcpPeerConnection> peerConn, std::size_t bytesRead) {
			// Call the application with the opaque connection wrapper and the amount of bytes read.
			auto streamConnection = make_shared<StreamConnection>(peerConn);
			serverReadStream(streamConnection, bytesRead);
		};

		_impl->AddTcpServer(port, std::move(readSome));
	}

	void IoServices::StartTcpServer(int port)
	{
		_impl->StartTcpServer(port);
	}


	/// <summary>
	/// 
	/// </summary>
	/// <param name="connectCb">Connect stream callback passed in by value to get a copy.</param>
	/// <param name="readCb">Read stream callback passed in by value to get a copy.</param>
	/// <param name="peerConn">The peer connection generated during the connection.</param>
	void ConnectHandler(ConnectStreamCallback connectCb, ReadStreamCallback readCb, std::shared_ptr<TcpPeerConnection> peerConn)
	{
		auto streamConnection = make_shared<StreamConnection>(peerConn);

		// Take copies of the stream connection and the callback to bind them to lambda.
		// The lambda then gets moved into the peer connection which ensures that they will be
		// associated with this peer connection and last the lifetime of the peer connection.
		auto available = [streamConnection, readCb](size_t available) {
			std::cout << "Client side GOT STUFF!!!!!: " << streamConnection->GetPeerEndPoint() << available << std::endl;
			readCb(streamConnection, available);
		};

		peerConn->BeginChainedRead(std::move(available), ReadSomeChunkSize);

		connectCb(streamConnection);
	}

	void IoServices::AsyncConnect(ConnectStreamCallback&& connectCb, ReadStreamCallback&& readCb, StreamErrorCallback&& errCb, std::string ipAddress, int port) {
		auto connect = bind(ConnectHandler, std::move(connectCb), std::move(readCb), std::placeholders::_1);

		auto cbCopy = move(errCb);
		auto error = [cbCopy, ipAddress, port](std::shared_ptr<TcpPeerConnection> conn, const boost::system::error_code& err) {
			stringstream errStr;
			errStr << "Client connection error on: " << ipAddress << ":" << port;
			cbCopy(errStr.str());
		};

		_impl->AsyncConnect(std::move(connect), std::move(error), ipAddress, port);
	}

	shared_ptr<DgramListener> IoServices::BindDgramListener(DgramReceiveCallback&& receiveCb, DgramErrorCallback&& errCb, const std::string& ipAddress, int port)
	{
		auto udpErrCb = [errCb](std::shared_ptr<UdpListener> listener, const boost::system::error_code& ec) {
			stringstream errStr;
			errStr << "Datagram listener error: " << listener->PeerEndPoint << ec << endl;
			errCb(errStr.str());
		};

		auto udpListener = _impl->BindDgramListener(move(udpErrCb), ipAddress, port);
		
		auto listener = make_shared<DgramListener>(udpListener);

		// NB injects a shared pointer to the datagram Listener into the UDP listener.
		// Which means that the UDP listener effectively owns the datagram listener.
		// So UDP listener only has a weak pointer to avoid the circular ownership problem.
		auto notify = [listener, receiveCb](size_t available) { // FIX ME do I need to move or this takes the copy?
			receiveCb(listener, available);
		};

		udpListener->BeginChainedRead(move(notify), 1470); // FIX ME datagram size constant.

		return listener;
	}

	void IoServices::SendToAllServerConnections(const std::string& msg, bool nullTerminate) {
		_impl->SendToAllServerConnections(msg, nullTerminate);
	}

	void IoServices::RunWork(std::function<void(void)>const& work) {
		_impl->RunWork(work);
	}

	void IoServices::SetPeriodicTimer(
		PeriodicTimer id,
		std::chrono::duration<long long>  du,
		const std::function<void(PeriodicTimer id)>& handler) {
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(du);
		auto ptimeFromNow = boost::posix_time::milliseconds(ms.count());
		_impl->SetPeriodicTimer(id, ptimeFromNow, handler);
	}

}