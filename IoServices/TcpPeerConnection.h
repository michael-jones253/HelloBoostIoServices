//
//  TcpPeerConnection.h
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__TcpPeerConnection__
#define __HelloAsio__TcpPeerConnection__

#include <boost/asio.hpp>

namespace HelloAsio {
    class TcpPeerConnection {
    private:
        boost::asio::ip::tcp::socket _peerSocket;
        boost::asio::ip::tcp::endpoint _peerEndPoint;

    public:
        TcpPeerConnection(
                          boost::asio::ip::tcp::socket&& peerSocket,
                          boost::asio::ip::tcp::endpoint&& peerEndPoint
        );
        
    };
}

#endif /* defined(__HelloAsio__TcpPeerConnection__) */
