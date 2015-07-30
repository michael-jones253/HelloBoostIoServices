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
        CloseAllPeerConnections();
    }
    
    void TcpServer::AcceptHandler(std::shared_ptr<TcpPeerConnection> acceptedConn, const boost::system::error_code& ec) {
        // FIX ME temp.
        // std::this_thread::sleep_for(std::chrono::seconds(4));
        
        if (ec != 0) {
            std::cerr << "Accept error: " << ec << std::endl;
        }
        
        _peerConnections.push_back(std::move(*acceptedConn));
        std::cout << "GOT A CONNECTION: " << _peerConnections.size() << std::endl;
        
        // Kick off another async accept to handle another connection.
        AsyncAccept();
    }
    
    void TcpServer::AsyncAccept() {
        auto conn = std::make_shared<TcpPeerConnection>(_ioService);

        auto acceptor = std::bind(&TcpServer::AcceptHandler, this, conn, std::placeholders::_1);

        std::cout << "ACCEPTING" << std::endl;
        
        // Async accept does not block and takes references to the socket and end point of the connection.
        // The connection smart pointer is kept alive by being bound to the acceptor callback.
        _acceptor->async_accept(conn->PeerSocket, conn->PeerEndPoint, std::move(acceptor));
    }
    
    void TcpServer::CloseAllPeerConnections() {
        for (auto& conn : _peerConnections) {
            conn.PeerSocket.close();
        }
    }
}
