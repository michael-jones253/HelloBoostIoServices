#include "IoServices.h"

#include "IoServicesImpl.h"
using namespace std;


namespace HelloAsio {
    IoServices::IoServices() : _impl(make_unique<IoServicesImpl>()) {
        _impl->Start();
    }
    
    IoServices::~IoServices() {
        
    }
    
    void IoServices::RunWork(std::function<void(void)>const& work) {
        _impl->RunWork(work);
    }

    void IoServices::SetPeriodicTimer(PeriodicTimer id, const std::function<void(PeriodicTimer id)>& handler) {
        auto ptimeFromNow = boost::posix_time::seconds(1);
        _impl->SetPeriodicTimer(id, ptimeFromNow, handler);
    }

}