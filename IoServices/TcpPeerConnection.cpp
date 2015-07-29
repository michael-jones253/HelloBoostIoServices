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
    TcpPeerConnection::TcpPeerConnection(
                  boost::asio::ip::tcp::socket&& peerSocket,
                  boost::asio::ip::tcp::endpoint&& peerEndPoint) :
    _peerSocket(std::move(peerSocket)),
    _peerEndPoint(std::move(peerEndPoint)) {
        
    }
}
