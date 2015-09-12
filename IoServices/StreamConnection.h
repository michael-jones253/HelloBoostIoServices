//
//  StreamConnection.h
//  HelloAsio
//
//  Created by Michael Jones on 29/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__StreamConnection__
#define __HelloAsio__StreamConnection__

#include <iostream>
#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace HelloAsio {
    // Hide Tcp peer connection.
    class TcpPeerConnection;
    class StreamConnection;
    
    using StreamErrorCallback = std::function<void(std::shared_ptr<StreamConnection>, std::string msg)>;
    
    using ReadStreamCallback = std::function<void(std::shared_ptr<StreamConnection>, int bytesAvailable)>;

	using ConnectStreamCallback = std::function<void(std::shared_ptr<StreamConnection>)>;

	struct EndPoint
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
	inline std::ostream& operator<<(std::ostream &os, const EndPoint &endPoint)
	{
		os << endPoint.IpAddress << ":" << endPoint.Port;

		return os;
	}

    
    class StreamConnection : public std::enable_shared_from_this<StreamConnection> {
    private:
        std::shared_ptr<TcpPeerConnection> _peerConnection;
        
    public:
        StreamConnection(std::shared_ptr<TcpPeerConnection> peerConnection);
        
        void ConsumeInto(uint8_t* buf, int len);
        
        void ExtractTo(std::vector<uint8_t>& dest, int len);
        
        const uint8_t* Data() const;
        
        size_t Size() const;
        
        void Consume(int len);

		EndPoint GetPeerEndPoint() const;
    };
}

#pragma GCC visibility pop


#endif /* defined(__HelloAsio__StreamConnection__) */
