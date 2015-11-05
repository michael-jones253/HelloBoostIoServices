//
//  TestTimer.h
//  AsyncIo
//
//  Created by Michael Jones on 28/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef HelloAsio_TestTimer_h
#define HelloAsio_TestTimer_h
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

#include "Timer.h"

namespace TestAsyncIo {
    
    enum class TestTimer : int {
        General,
        End
    };

	std::string ToString(TestTimer t)
	{
		switch (t)
		{
		case TestTimer::General:
			return "TestTimer::General";

		case TestTimer::End:
			return "TestTimer::End";

		default:
			throw std::runtime_error("Unknown TestTimer");
		}
	}

	AsyncIo::Timer ToId(TestTimer t)
	{
		return AsyncIo::Timer{static_cast<int>(t), ToString(t)};
	}

    inline TestTimer& operator++(TestTimer& rhs) {
        rhs = static_cast<TestTimer>(static_cast<int>(rhs) + 1);
        return rhs;
    }

	inline std::ostream& operator<<(std::ostream& os, TestTimer rhs) {
		switch (rhs) {
		case TestTimer::General:
			os << "TestTimer::General";
			break;
		case TestTimer::End:
			os << "TestTimer::End";
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
