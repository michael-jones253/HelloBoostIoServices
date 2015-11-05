//
//  IoBufferWrapper.cpp
//  AsyncIo
//
//  Created by Michael Jones on 31/07/2015.
//   https://github.com/michael-jones253/HelloBoostIoServices
//
#include "stdafx.h"
#include "IoBufferWrapper.h"
using namespace std;

namespace AsyncIo {
	IoBufferWrapper::IoBufferWrapper(const std::string& msg, bool nullTerminate) :
		NullTerminate{ nullTerminate } {
        Buffer.insert(Buffer.begin(), msg.cbegin(), msg.cend());

		if (nullTerminate) {
			Buffer.push_back('\0');
		}
	}

	IoBufferWrapper::IoBufferWrapper(std::string&& msg, bool nullTerminate) :
		NullTerminate{ nullTerminate }
	{
		StringBuffer = move(msg);
	}

    
    IoBufferWrapper::IoBufferWrapper(vector<uint8_t>&& rhs) :
		StringBuffer{} {
        Buffer = move(rhs);
	}
    
    IoBufferWrapper::IoBufferWrapper(IoBufferWrapper&& rhs) {
        Buffer = move(rhs.Buffer);
		StringBuffer = move(rhs.StringBuffer);
		NullTerminate = rhs.NullTerminate;
	}
    
    IoBufferWrapper& IoBufferWrapper::operator=(IoBufferWrapper&& rhs) {
        Buffer = move(rhs.Buffer);
		StringBuffer = move(rhs.StringBuffer);
		NullTerminate = rhs.NullTerminate;
        return *this;
    }

	boost::asio::const_buffers_1 IoBufferWrapper::ToBoost() {
		if (!StringBuffer.empty()) {
			auto len = NullTerminate ? StringBuffer.size() + 1 : StringBuffer.size();
			const auto bufPtr = reinterpret_cast<const void*>(StringBuffer.data());
			return boost::asio::buffer(bufPtr, len);
		}
		else {
			const auto bufPtr = reinterpret_cast<const void*>(Buffer.data());
			return boost::asio::buffer(bufPtr, Buffer.size());
		}
	}

	size_t IoBufferWrapper::BoostSize() const
	{
		if (!StringBuffer.empty()) {
			return NullTerminate ? StringBuffer.size() + 1 : StringBuffer.size();
		}
		else
		{
			return Buffer.size();
		}
	}


}
