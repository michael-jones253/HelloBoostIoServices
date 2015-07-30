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
    struct TcpPeerConnection {
        boost::asio::ip::tcp::socket PeerSocket;
        boost::asio::ip::tcp::endpoint PeerEndPoint;

        TcpPeerConnection(boost::asio::io_service* ioService);
        
        TcpPeerConnection(TcpPeerConnection&& rhs);
    };
}

#endif /* defined(__HelloAsio__TcpPeerConnection__) */
