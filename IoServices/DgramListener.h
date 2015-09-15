//
//  DgramListener.h
//  HelloAsio
//
//  Created by Michael Jones on 29/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__DgramListener__
#define __HelloAsio__DgramListener__

#include <iostream>
#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace HelloAsio {
    // Hide UDP listener.
    class UdpListener;
    class DgramListener;
    
    using DgramErrorCallback = std::function<void(const std::string& msg)>;
    
    using DgramReceiveCallback = std::function<void(std::shared_ptr<DgramListener>, int bytesAvailable)>;

	// FIX ME - use this for TCP and put in a separate file.
	struct IoEndPoint
	{
		std::string IpAddress;
		int Port;
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

    
    class DgramListener : public std::enable_shared_from_this<DgramListener> {
    private:
        std::weak_ptr<UdpListener> _udpListener;
        
    public:
		DgramListener(std::shared_ptr<UdpListener> udpListener);

		void AsyncWrite(std::string&& msg, bool nullTerminate);
        
        void ConsumeInto(uint8_t* buf, int len);
        
        void ExtractTo(std::vector<uint8_t>& dest, int len);
        
        const uint8_t* Data() const;
        
        size_t Size() const;
        
        void Consume(int len);

		IoEndPoint GetPeerEndPoint() const;
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__DgramListener__) */
