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
#include <array>
#include <memory>

namespace boost {
    namespace asio {
        namespace ip {
            class udp;
            template <typename T>
            class basic_endpoint;
            typedef basic_endpoint<udp> endpoint;
        }
    }
}

namespace AsyncIo
{
	class IoEndPointImpl;

	/// <summary>
	/// For UDP use only. I should rename this accordingly.
	/// </summary>
	class IoEndPoint
	{
	friend std::ostream& operator<<(std::ostream &os, const IoEndPoint &endPoint);
	friend std::wostream& operator<<(std::wostream &os, const IoEndPoint &endPoint);

    private:
        std::unique_ptr<IoEndPointImpl> _impl;

    public:

		std::string IpAddress() const;
		int Port() const;
        std::array<uint8_t, 4> IpV4NetworkBytes() const;
        uint32_t IpV4HostBytes() const;
        bool IsBroadcast() const;

		IoEndPoint();
		IoEndPoint(const IoEndPoint& rhs);
		IoEndPoint(IoEndPoint&& rhs);
		IoEndPoint& operator=(IoEndPoint&& rhs);

		IoEndPoint(const std::string& ipAddress, int port);

        ~IoEndPoint();

        // For use by low level Ioservices only.
		IoEndPoint(const boost::asio::ip::endpoint& boostEp, bool asyncConnected, bool asyncReceivedFrom);
        const boost::asio::ip::endpoint& ToBoost() const;

    private:
        bool HasAsyncConnected() const;
        bool HasAsyncReceivedFrom() const;
	};

	/// <summary>
	/// For logging and error reporting.
	/// </summary>
	/// <param name="os">Output stream.</param>
	/// <param name="groupType">The end point.</param>
	/// <returns>End point as readable text stream.</returns>
	inline std::ostream& operator<<(std::ostream &os, const IoEndPoint &endPoint)
	{
		os << endPoint.IpAddress() << ":" << endPoint.Port()
        << " connected: " << (endPoint.HasAsyncConnected() ? "true" : "false")
        << " received from: " << (endPoint.HasAsyncReceivedFrom() ? "true" : "false");

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
        auto addrStr = endPoint.IpAddress();
		os << std::wstring(addrStr.begin(), addrStr.end()) << ":" << endPoint.Port()
        << " connected: " << (endPoint.HasAsyncConnected() ? "true" : "false")
        << " received from: " << (endPoint.HasAsyncReceivedFrom() ? "true" : "false");

		return os;
	}
}

#endif
