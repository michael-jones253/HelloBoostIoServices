//
//  IoEndPoint.h
//  AsyncIo
//
//  Created by Michael Jones on 17/06/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__IoEndPoint__
#define __HelloAsio__IoEndPoint__

#include <ostream>
#include <string>

namespace AsyncIo
{
	/// <summary>
	/// For UDP use only. I should rename this accordingly.
	/// </summary>
	struct IoEndPoint
	{
		std::string IpAddress;
		int Port;
		bool AsyncConnected;

		IoEndPoint() :
			IpAddress(),
			Port{},
			AsyncConnected{}
		{
		}

		IoEndPoint(const std::string ipAddress, int port, bool asyncConnected) :
			IpAddress(ipAddress),
			Port{ port },
			AsyncConnected{ asyncConnected }
		{
		}
	};

	/// <summary>
	/// For logging and error reporting.
	/// </summary>
	/// <param name="os">Output stream.</param>
	/// <param name="groupType">The end point.</param>
	/// <returns>End point as readable text stream.</returns>
	inline std::ostream& operator<<(std::ostream &os, const IoEndPoint &endPoint)
	{
		os << endPoint.IpAddress << ":" << endPoint.Port;

		return os;
	}

	/// <summary>
	/// For logging and error reporting.
	/// </summary>
	/// <param name="os">Output stream.</param>
	/// <param name="groupType">The end point.</param>
	/// <returns>End point as readable text stream.</returns>
	inline std::wostream& operator<<(std::wostream &os, const IoEndPoint &endPoint)
	{
		os << std::wstring(endPoint.IpAddress.begin(), endPoint.IpAddress.end()) << ":" << endPoint.Port;

		return os;
	}
}

#endif
