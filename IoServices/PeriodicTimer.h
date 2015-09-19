//
//  PeriodicTimer.h
//  AsyncIo
//
//  Created by Michael Jones on 28/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef HelloAsio_PeriodicTimer_h
#define HelloAsio_PeriodicTimer_h

#include <ostream>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo {
    
    enum class PeriodicTimer : int {
        General,
        Secondary,
        End
    };

    inline PeriodicTimer operator++(PeriodicTimer& rhs) {
        rhs = static_cast<PeriodicTimer>(static_cast<int>(rhs) + 1);
        return rhs;
    }

    inline std::ostream& operator<<(std::ostream& os, PeriodicTimer rhs) {
        switch (rhs) {
            case PeriodicTimer::General:
                os << "PeriodicTimer::General";
                break;
            case PeriodicTimer::Secondary:
                os << "PeriodicTimer::Secondary";
                break;
            case PeriodicTimer::End:
                os << "PeriodicTimer::End";
                break;
            default:
                break;
        }
        
        return os;
    }

}

#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif
