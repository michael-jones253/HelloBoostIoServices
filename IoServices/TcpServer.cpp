//
//  TcpServer.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "TcpServer.h"

#include <iostream>

namespace HelloAsio {
    TcpServer::TcpServer(boost::asio::io_service* ioService, int port) :
    _ioService{ioService},
    _port{port},
    _acceptor{},
    _peerConnections{}
    {
    }
    
    void TcpServer::Start() {
        _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                                                             *_ioService,
                                                             boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                             _port));
        AsyncAccept();
    }
    
    void TcpServer::AsyncAccept() {
        boost::asio::ip::tcp::endpoint peerEndPoint{};
        boost::asio::ip::tcp::socket peerSocket(*_ioService);

        auto handler = [&](const boost::system::error_code& error) {
            _peerConnections.emplace_back(std::move(peerSocket), std::move(peerEndPoint));
            std::cout << "GOT A CONNECTION: " << _peerConnections.size() << std::endl;

            // Kick off another async accept to handle another connection.
            AsyncAccept();
        };

        std::cout << "ACCEPTING" << std::endl;
        _acceptor->async_accept(peerSocket, peerEndPoint, std::move(handler));
    }
}
