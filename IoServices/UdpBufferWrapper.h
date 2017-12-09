//
//  DgramBufferWrapper.h
//  AsyncIo
//
//  Created by Michael Jones on 17/06/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__UdpBufferWrapper__
#define __HelloAsio__UdpBufferWrapper__

#include "IoBufferWrapper.h"
#include "IoEndPoint.h"
#include <memory>

namespace AsyncIo
{
	class UdpBufferWrapper
	{
	private:
		IoBufferWrapper _bufferWrapper;
		std::unique_ptr<IoEndPoint> _destEndPoint;

	public:
		UdpBufferWrapper(const std::string& msg, bool nullTerminate);
		UdpBufferWrapper(std::string&& msg, bool nullTerminate);
		UdpBufferWrapper(std::string&& msg, const std::string& destIp, int port, bool nullTerminate);
		UdpBufferWrapper(std::vector<uint8_t>&& rhs);
		UdpBufferWrapper(std::vector<uint8_t>&& rhs, const IoEndPoint& dest);

		UdpBufferWrapper(UdpBufferWrapper&& rhs);
		UdpBufferWrapper& operator=(AsyncIo::UdpBufferWrapper&& rhs);

		boost::asio::const_buffers_1 ToBoost();
		size_t BoostSize() const;
		IoEndPoint* DestEp() const;
	};
}

#endif
