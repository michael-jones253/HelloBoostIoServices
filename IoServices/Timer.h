//
//  Timer.h
//  HelloAsio
//
//  Created by Michael Jones on 28/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef HelloAsio_Timer_h
#define HelloAsio_Timer_h

/* The classes below are exported */
#pragma GCC visibility push(default)
namespace HelloAsio {
    
    enum class Timer : int {
        General,
        End
    };

    inline Timer& operator++(Timer& rhs) {
        rhs = static_cast<Timer>(static_cast<int>(rhs) + 1);
        return rhs;
    }
}

#pragma GCC visibility pop

#endif
