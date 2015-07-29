#include "IoServices.h"

#include "IoServicesImpl.h"

namespace HelloAsio {
    IoServices::IoServices() : _impl(std::make_unique<IoServicesImpl>()) {
        _impl->Start();
    }
    
    IoServices::~IoServices() {
        
    }
    
    void IoServices::RunTcpServer(int port) {
        _impl->RunTcpServer(port);
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