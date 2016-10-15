#include "stdafx.h"
#include "UdpBufferWrapper.h"

using namespace std;

namespace AsyncIo
{
	UdpBufferWrapper::UdpBufferWrapper(const std::string& msg, bool nullTerminate) :
		_bufferWrapper(msg, nullTerminate),
		_destEndPoint{}
	{

	}

	UdpBufferWrapper::UdpBufferWrapper(std::string&& msg, bool nullTerminate) :
		_bufferWrapper(move(msg), nullTerminate),
		_destEndPoint{}
	{

	}

	UdpBufferWrapper::UdpBufferWrapper(std::string&& msg, const std::string& destIp, int port, bool nullTerminate) :
		_bufferWrapper(move(msg), nullTerminate),
		_destEndPoint{ make_unique<IoEndPoint>( destIp, port, false ) }
	{

	}

	UdpBufferWrapper::UdpBufferWrapper(std::vector<uint8_t>&& rhs) :
		_bufferWrapper(move(rhs)),
		_destEndPoint{}
	{

	}

	UdpBufferWrapper::UdpBufferWrapper(UdpBufferWrapper&& rhs) :
		_bufferWrapper(move(rhs._bufferWrapper)),
		_destEndPoint{}
	{

	}

	UdpBufferWrapper& UdpBufferWrapper::operator=(AsyncIo::UdpBufferWrapper&& rhs)
	{
		_bufferWrapper = move(rhs._bufferWrapper);
		_destEndPoint = move(rhs._destEndPoint);

		return *this;
	}

	boost::asio::const_buffers_1 UdpBufferWrapper::ToBoost()
	{
		return _bufferWrapper.ToBoost();
	}

	size_t UdpBufferWrapper::BoostSize() const
	{
		return _bufferWrapper.BoostSize();
	}

	IoEndPoint* UdpBufferWrapper::DestEp() const
	{
		return _destEndPoint.get();
	}

}