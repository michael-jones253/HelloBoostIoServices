//
//  IoBufferWrapper.h
//  AsyncIo
//
//  Created by Michael Jones on 31/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__IoBufferWrapper__
#define __HelloAsio__IoBufferWrapper__

#include <string>
#include <vector>
#include <cstdint>

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo
{
	/// <summary>
	/// Convenience wrapper for constructing a message to send.
	/// </summary>
    struct IoBufferWrapper
	{
        std::vector<uint8_t> Buffer;
        
        IoBufferWrapper(const std::string& msg, bool nullTerminate);
        IoBufferWrapper(std::vector<uint8_t>&& rhs);
        
        IoBufferWrapper(IoBufferWrapper&& rhs);
        IoBufferWrapper& operator=(AsyncIo::IoBufferWrapper&& rhs);

    };
}

#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__IoBufferWrapper__) */
