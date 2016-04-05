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

#include <string>
#include <functional>

namespace AsyncIo {
    
	struct PeriodicTimer final {
		const int Id;
		const std::string Name;

		bool operator<(const PeriodicTimer& rhs) const;
		bool operator==(const PeriodicTimer& rhs) const;
	};

	/// <summary>
	/// For map key operations.
	/// </summary>
	inline bool PeriodicTimer::operator<(const PeriodicTimer& rhs) const {
		return Id < rhs.Id;
	}

	inline bool PeriodicTimer::operator==(const PeriodicTimer& rhs) const {
		return Id == rhs.Id;
	}

	/// <summary>
	/// For unordered map.
	/// </summary>
	struct PeriodicTimerHasher
	{
		std::size_t operator()(const PeriodicTimer& k) const
		{
			using std::size_t;
			using std::hash;

			// Operate on integer id only, for simplicity and efficiency.
			return hash<int>()(k.Id);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, PeriodicTimer rhs) {
		os << rhs.Name;
		return os;
	}

}

#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif
