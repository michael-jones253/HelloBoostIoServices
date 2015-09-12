//
//  TcpServer.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "stdafx.h"
#include "TcpServer.h"

#include <iostream>
#include <thread>
#include <boost/bind.hpp>

namespace
{
	const int ChunkSize = 12;
}

namespace HelloAsio {
    TcpServer::TcpServer(boost::asio::io_service* ioService, int port, ReadSomeCallback&& readSomeCb) :
    _ioService{ioService},
    _port{port},
    _acceptor{},
    _peerConnections{},
    _readSomeCb{ std::move(readSomeCb) }
    {
    }
    
    TcpServer::TcpServer(TcpServer&& rhs) {
        _ioService = rhs._ioService;
        _port = rhs._port;
        _acceptor = std::move(rhs._acceptor);
        _peerConnections = std::move(rhs._peerConnections);
        _readSomeCb = std::move(rhs._readSomeCb);
    }

    TcpServer::~TcpServer() {
        Stop();
    }
    
    void TcpServer::Start() {
        _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                                                             *_ioService,
                                                             boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _port));
        AsyncAccept();
    }
    
    void TcpServer::Stop() {
        _acceptor->close();
        CloseAllPeerConnections();
    }
    
    void TcpServer::AcceptHandler(std::shared_ptr<TcpPeerConnection> acceptedConn, const boost::system::error_code& ec) {
        if (ec != 0) {
            std::cerr << "Accept error: " << ec << std::endl;
        }
        
        auto available = [this, acceptedConn](size_t available) {
            std::cout << "GOT STUFF!!!!!: " << acceptedConn->PeerEndPoint << available << std::endl;
            _readSomeCb(acceptedConn, available);
        };
        
        acceptedConn->BeginChainedRead(std::move(available), ChunkSize);
        
		std::lock_guard<std::mutex> connGuard(_mutex);
        _peerConnections.push_back(acceptedConn);
        std::cout << "GOT A CONNECTION: " << _peerConnections.size() << std::endl;
        
        // Kick off another async accept to handle another connection.
        AsyncAccept();
    }
    
    void TcpServer::AsyncAccept() {
        auto errorHandler = std::bind(&TcpServer::ErrorHandler, this, std::placeholders::_1, std::placeholders::_2);
        
        auto conn = std::make_shared<TcpPeerConnection>(_ioService, std::move(errorHandler));

        auto acceptor = std::bind(&TcpServer::AcceptHandler, this, conn, std::placeholders::_1);

        std::cout << "ACCEPTING" << std::endl;

        // Async accept does not block and takes references to the socket and end point of the connection.
        // The connection smart pointer is kept alive by being bound to the acceptor callback.
        _acceptor->async_accept(conn->PeerSocket, conn->PeerEndPoint, std::move(acceptor));
    }
    
    void TcpServer::CloseAllPeerConnections() {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto& conn : _peerConnections) {
            conn->PeerSocket.close();
        }
    }

    void TcpServer::WriteHandlerDeprecated(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec, std::size_t written) {
        if (ec != 0) {
            std::cerr << "Write error" << std::endl;
            
            std::lock_guard<std::mutex> guard(_mutex);
            conn->PeerSocket.close();
            for (auto pos = _peerConnections.begin(); pos != _peerConnections.end(); pos++) {
                if (pos->get()->PeerSocket.is_open()) {
                    continue;
                }
                
                std::cerr << "ERASING PEER: " << pos->get()->PeerEndPoint.address() << std::endl;
                _peerConnections.erase(pos);
                break;
            }
        }
    }
    
    void TcpServer::ErrorHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec) {
        if (ec != 0) {
            std::cerr << "Write error" << std::endl;
            
            std::lock_guard<std::mutex> guard(_mutex);
            conn->PeerSocket.close();
            for (auto pos = _peerConnections.begin(); pos != _peerConnections.end(); pos++) {
                if (pos->get()->PeerSocket.is_open()) {
                    continue;
                }
                
                std::cerr << "ERASING PEER: " << pos->get()->PeerEndPoint.address() << std::endl;
                _peerConnections.erase(pos);
                break;
            }
        }
    }
    
    void TcpServer::SendMessageToAllPeersDeprecated(const std::string& msg) {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto& conn : _peerConnections) {
            auto handler = std::bind(&TcpServer::WriteHandlerDeprecated, this, conn, std::placeholders::_1, std::placeholders::_2);
            boost::asio::async_write(conn->PeerSocket, boost::asio::buffer(msg), std::move(handler));
        }
    }

    void TcpServer::SendMessageToAllPeers(const std::string& msg) {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto& conn : _peerConnections) {

            // Allocate message for move.
            auto msgBuf = msg;
            conn->AsyncWrite(std::move(msgBuf));
        }
    }
    
}
