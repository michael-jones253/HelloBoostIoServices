//
//  IoEndPoint.cpp
//  AsyncIo
//
//  Created by Michael Jones on 08/04/2017.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#include "IoEndPoint.h"

#include <boost/asio.hpp>

namespace AsyncIo
{
	class IoEndPointImpl
	{
    private:
        boost::asio::ip::udp::endpoint _boostEndPoint;
        bool _isBroadcast{};
        bool _asyncConnected{};
        bool _asyncReceivedFrom{};

    public:
        IoEndPointImpl(const std::string& ipAddress, int port) :
            _isBroadcast{ ipAddress == "255.255.255.255" },
            _boostEndPoint{ _isBroadcast ?
                    boost::asio::ip::address_v4::broadcast()
                    : boost::asio::ip::address::from_string(ipAddress),
                            static_cast<unsigned short>(port) }
        {
        }

        IoEndPointImpl(const IoEndPointImpl& rhs) :
            _isBroadcast{ rhs._isBroadcast },
            _boostEndPoint{ rhs._boostEndPoint }
        {
        }

        IoEndPointImpl(const boost::asio::ip::endpoint& boostEp, bool asyncConnected, bool asyncReceivedFrom) :
            _isBroadcast{ /* FIXME */ },
            _boostEndPoint{ boostEp },
            _asyncConnected{ asyncConnected },
            _asyncReceivedFrom{ asyncReceivedFrom }
        {
        }

        const boost::asio::ip::endpoint& ToBoost() const {
            return  _boostEndPoint;
        }

        bool IsBroadcast() const {
            return _isBroadcast;
        }

        bool HasAsyncConnected() const {
            return _asyncConnected;
        }

        bool HasAsyncReceivedFrom() const {
            return _asyncReceivedFrom;
        }

	};

    IoEndPoint::IoEndPoint() :
        _impl{}
    {
    }

    IoEndPoint::IoEndPoint(const IoEndPoint& rhs) :
        _impl{ std::make_unique<IoEndPointImpl>(*(rhs._impl)) }
    {
    }

    IoEndPoint::IoEndPoint(IoEndPoint&& rhs) :
         _impl{ move(rhs._impl) }
    {
    }

    IoEndPoint::IoEndPoint(const std::string& ipAddress, int port) :
        _impl{ std::make_unique<IoEndPointImpl>(ipAddress, port) }
    {
    }

    IoEndPoint::IoEndPoint(const boost::asio::ip::endpoint& boostEp, bool asyncConnected, bool asyncReceivedFrom) :
        _impl{ std::make_unique<IoEndPointImpl>(boostEp, asyncConnected, asyncReceivedFrom) }
    {
    }

    IoEndPoint::~IoEndPoint()
    {
    }

    const boost::asio::ip::endpoint& IoEndPoint::ToBoost() const {
        return _impl->ToBoost();
    }

    std::string IoEndPoint::IpAddress() const {
        boost::system::error_code ec;
        auto addrStr = _impl->ToBoost().address().to_string(ec);
        if (ec.value() != 0) {
            return std::string("bad IP address: ") + ec.message();
        }

        return addrStr;
    }

    int IoEndPoint::Port() const {
        return _impl->ToBoost().port();
    }

    std::array<uint8_t, 4> IoEndPoint::IpV4NetworkBytes() const {
        return _impl->ToBoost().address().to_v4().to_bytes();
    }

    uint32_t IoEndPoint::IpV4HostBytes() const {
        return _impl->ToBoost().address().to_v4().to_ulong();
    }

    bool IoEndPoint::IsBroadcast() const {
        return _impl->IsBroadcast();
    }

    bool IoEndPoint::HasAsyncConnected() const {
        return _impl->HasAsyncConnected();
    }

    bool IoEndPoint::HasAsyncReceivedFrom() const {
        return _impl->HasAsyncReceivedFrom();
    }
}

