//
//  TcpSslConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 14/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

#include "TcpSslConnection.hpp"

#include <iostream>

namespace AsyncIo {
    TcpSslConnection::TcpSslConnection(boost::asio::io_service* ioService,
                                       boost::asio::ssl::context&& ctx,
                                       AsyncIo::ErrorCallback&& errorCallback) :
    PeerSocket{*ioService},
    PeerEndPoint{},
    Ctx(std::move(ctx)),
    // Ctx(boost::asio::ssl::context::sslv23),
    SslSocket(PeerSocket, Ctx),
    _mutex{},
    mOutQueue{},
    _errorCallback{ std::move(errorCallback) },
    _readBuffer{}
    {
    
    }
    
    void TcpSslConnection::AsyncConnect(SslConnectCallback&& connCb,
                                        std::string ipAddress, int port) {
        // SSL stuff.
        SslSocket.set_verify_mode(boost::asio::ssl::verify_peer);
        SslSocket.set_verify_callback(
                                      std::bind(&TcpSslConnection::VerifyCertificate, this,
                                                std::placeholders::_1, std::placeholders::_2));
        
        // Basic socket connect stuff.
        boost::asio::ip::tcp::endpoint remoteEp{ boost::asio::ip::address::from_string(ipAddress), static_cast<unsigned short>(port) };
        _connectCallback = std::move(connCb);
        
        auto boostHandler = std::bind(&TcpSslConnection::ConnectHandler, this, shared_from_this(),std::placeholders::_1);
        PeerEndPoint = std::move(remoteEp);
        PeerSocket.async_connect(PeerEndPoint, std::move(boostHandler));
    }
    
    void TcpSslConnection::ConnectHandler(std::shared_ptr<TcpSslConnection> conn, boost::system::error_code ec) {
        if (ec)
        {
            _errorCallback(conn, ec);
            return;
        }
        
        SslSocket.async_handshake(boost::asio::ssl::stream_base::client,
                                std::bind(&TcpSslConnection::HandleHandshake, this,
                                            std::placeholders::_1));


    }
    
    bool TcpSslConnection::VerifyCertificate(bool preverified,
                           boost::asio::ssl::verify_context& ctx) {
        std::cout << "Verifying certificate, pre-verified: " << std::string(preverified ? "true" : "false");
        // FIX ME - verify.
        return preverified;
    }

    
    void TcpSslConnection::HandleHandshake(const boost::system::error_code& ec) {
        if (ec)
        {
            _errorCallback(shared_from_this(), ec);
            return;
        }
        
        _connectCallback(shared_from_this());

    }
}