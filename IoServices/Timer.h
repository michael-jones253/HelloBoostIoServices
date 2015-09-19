//
//  Timer.h
//  AsyncIo
//
//  Created by Michael Jones on 28/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef HelloAsio_Timer_h
#define HelloAsio_Timer_h
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo {
    
    enum class Timer : int {
        General,
        End
    };

    inline Timer& operator++(Timer& rhs) {
        rhs = static_cast<Timer>(static_cast<int>(rhs) + 1);
        return rhs;
    }
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif
