//
//  PeriodicTimer.h
//  HelloAsio
//
//  Created by Michael Jones on 28/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef HelloAsio_PeriodicTimer_h
#define HelloAsio_PeriodicTimer_h

#include <ostream>

namespace HelloAsio {
    
    enum class PeriodicTimer : int {
        General,
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
            case PeriodicTimer::End:
                os << "PeriodicTimer::End";
                break;
            default:
                break;
        }
        
        return os;
    }

}

#endif
