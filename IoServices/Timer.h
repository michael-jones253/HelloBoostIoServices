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

#include <string>
#include <functional>

namespace AsyncIo {
    
    struct Timer final {
		const int Id;
		const std::string Name;

		bool operator<(const Timer& rhs) const;
		bool operator==(const Timer& rhs) const;
	};

	/// <summary>
	/// For map key operations.
	/// </summary>
	inline bool Timer::operator<(const Timer& rhs) const {
        return Id < rhs.Id;
    }

	inline bool Timer::operator==(const Timer& rhs) const {
		return Id == rhs.Id;
	}

	/// <summary>
	/// For unordered map.
	/// </summary>
	struct TimerHasher
	{
		std::size_t operator()(const Timer& k) const
		{
			using std::size_t;
			using std::hash;

			// Operate on integer id only, for simplicity and efficiency.
			return hash<int>()(k.Id);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, Timer rhs) {
		os << rhs.Name;
		return os;
	}

}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif
