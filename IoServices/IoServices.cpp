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

	void IoServices::RunTcpServer(int port, ReadStreamCallback&& readStream) {
		_serverReadStream = move(readStream);

		auto readSome = [this](std::shared_ptr<TcpPeerConnection> peerConn, std::size_t bytesRead) {
			// Call the application with the opaque connection wrapper and the amount of bytes read.
			auto streamConnection = make_shared<StreamConnection>(peerConn);
			_serverReadStream(streamConnection, bytesRead);
		};

		_impl->RunTcpServer(port, std::move(readSome));
	}

	void ConnectHandler(ConnectStreamCallback connectCb, ReadStreamCallback readCb, std::shared_ptr<TcpPeerConnection> peerConn)
	{
		auto streamConnection = make_shared<StreamConnection>(peerConn);

		auto available = [streamConnection, readCb](size_t available) {
			std::cout << "Client side GOT STUFF!!!!!: " << streamConnection->GetPeerEndPoint() << available << std::endl;
			readCb(streamConnection, available);
		};

		peerConn->BeginChainedRead(std::move(available), ReadSomeChunkSize);

		connectCb(streamConnection);
	}

	void IoServices::AsyncConnect(ConnectStreamCallback&& connectCb, ReadStreamCallback&& readCb, std::string ipAddress, int port) {
		auto connect = bind(ConnectHandler, std::move(connectCb), std::move(readCb), std::placeholders::_1);
		_impl->AsyncConnect(std::move(connect), ipAddress, port);
	}

	void IoServices::HelloAllPeers() {
		_impl->HelloAllPeers();
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