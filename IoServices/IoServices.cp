#include "IoServices.h"

#include "IoServicesImpl.h"

using namespace std;

namespace HelloAsio {
    IoServices::IoServices() : _impl(std::make_unique<IoServicesImpl>()) {
        _impl->Start();
    }
    
    IoServices::~IoServices() {
        
    }
    
    void IoServices::RunTcpServer(int port, ReadStreamCallback&& readStream) {
        _readStream = move(readStream);
        
        auto readSome = [this](std::shared_ptr<TcpPeerConnection> peerConn, std::size_t bytesRead) {
            // Call the application with the opaque connection wrapper and the amount of bytes read.
            auto streamConnection = make_shared<StreamConnection>(peerConn);
            _readStream(streamConnection, bytesRead);
        };
        
        _impl->RunTcpServer(port, std::move(readSome));
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