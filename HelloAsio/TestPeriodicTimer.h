//
//  TestPeriodicTimer.h
//  AsyncIo
//
//  Created by Michael Jones on 28/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef HelloAsio_TestPeriodicTimer_h
#define HelloAsio_TestPeriodicTimer_h

#include <ostream>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

#include "PeriodicTimer.h"

namespace TestAsyncIo {
    
    enum class TestPeriodicTimer : int {
        General,
        Secondary,
        End
    };

	std::string ToString(TestPeriodicTimer t)
	{
		switch (t)
		{
		case TestPeriodicTimer::General:
			return "TestPeriodicTimer::General";

		case TestPeriodicTimer::Secondary:
			return "TestPeriodicTimer::Secondary";

		case TestPeriodicTimer::End:
			return "TestTimer::End";

		default:
			throw std::runtime_error("Unknown TestPeriodicTimer");
		}
	}

	AsyncIo::PeriodicTimer ToId(TestPeriodicTimer t)
	{
		return AsyncIo::PeriodicTimer{ static_cast<int>(t), ToString(t) };
	}


    inline TestPeriodicTimer operator++(TestPeriodicTimer& rhs) {
        rhs = static_cast<TestPeriodicTimer>(static_cast<int>(rhs) + 1);
        return rhs;
    }

    inline std::ostream& operator<<(std::ostream& os, TestPeriodicTimer rhs) {
        switch (rhs) {
            case TestPeriodicTimer::General:
                os << "TestPeriodicTimer::General";
                break;
            case TestPeriodicTimer::Secondary:
                os << "TestPeriodicTimer::Secondary";
                break;
            case TestPeriodicTimer::End:
                os << "TestPeriodicTimer::End";
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
