//
//  TcpConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "TcpPeerConnection.h"
namespace HelloAsio
{
    TcpPeerConnection::TcpPeerConnection(boost::asio::io_service* ioService) :
        PeerSocket{*ioService},
        PeerEndPoint{} {
        
    }
    
    TcpPeerConnection::TcpPeerConnection(TcpPeerConnection&& rhs) :
        PeerSocket(std::move(rhs.PeerSocket)),
        PeerEndPoint(std::move(rhs.PeerEndPoint)) {
    }

}
