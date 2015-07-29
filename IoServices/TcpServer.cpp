//
//  TcpServer.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "TcpServer.h"

#include <iostream>
#include <thread>

namespace HelloAsio {
    TcpServer::TcpServer(boost::asio::io_service* ioService, int port) :
    _ioService{ioService},
    _port{port},
    _acceptor{},
    _peerConnections{}
    {
    }
    
    TcpServer::TcpServer(TcpServer&& rhs) {
        _ioService = rhs._ioService;
        _port = rhs._port;
        _acceptor = std::move(rhs._acceptor);
        _peerConnections = std::move(rhs._peerConnections);
    }

    TcpServer::~TcpServer() {
        Stop();
    }
    
    void TcpServer::Start() {
        _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                                                             *_ioService,
                                                             boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                             _port));
        AsyncAccept();
    }
    
    void TcpServer::Stop() {
        _acceptor->close();
    }
    
    void TcpServer::AcceptHandler(std::shared_ptr<TcpPeerConnection> acceptedConn, const boost::system::error_code& error) {
        // FIX ME temp.
        std::this_thread::sleep_for(std::chrono::seconds(4));
        
        _peerConnections.emplace_back(std::move(acceptedConn->PeerSocket), std::move(acceptedConn->PeerEndPoint));
        std::cout << "GOT A CONNECTION: " << _peerConnections.size() << std::endl;
        
        // Kick off another async accept to handle another connection.
        AsyncAccept();
    }
    
    void TcpServer::AsyncAccept() {
        // FIX ME - constructor which takes io service.
        boost::asio::ip::tcp::endpoint peerEndPoint{};
        boost::asio::ip::tcp::socket peerSocket(*_ioService);
        auto conn = std::make_shared<TcpPeerConnection>(std::move(peerSocket), std::move(peerEndPoint));

        auto acceptor = std::bind(&TcpServer::AcceptHandler, this, conn, std::placeholders::_1);

        std::cout << "ACCEPTING" << std::endl;
        // Async accept takes references to the socket and end point, which are kept alive by the smart
        // pointer bound to the acceptor callback.
        _acceptor->async_accept(conn->PeerSocket, conn->PeerEndPoint, std::move(acceptor));
    }
}
