//
//  TcpSslConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 14/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"
#include "TcpSslConnection.hpp"

#include <iostream>

namespace AsyncIo {
    TcpSslConnection::TcpSslConnection(boost::asio::io_service* ioService,
                                       boost::asio::ssl::context&& ctx,
                                       AsyncIo::ErrorCallback&& errorCallback) :
    TcpPeerConnection(ioService, std::move(errorCallback)),
    Ctx(std::move(ctx)),
    // Ctx(boost::asio::ssl::context::sslv23),
    SslSocket(PeerSocket, Ctx)
    {
    }
    
    void TcpSslConnection::AsyncWriteToSocket(std::shared_ptr<IoBufferWrapper>& bufferWrapper, BoostIoHandler&& handler) {
        boost::asio::async_write(SslSocket, bufferWrapper->ToBoost(), std::move(handler));
    }
    
    void TcpSslConnection::AsyncReadSome(uint8_t* bufPtr, size_t len, BoostIoHandler&& handler) {
        // Unless buffer is created with a mutable pointer the boost buffer will not be mutable.
        auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(bufPtr), len);
        boost::asio::async_read(SslSocket, boostBuf, boost::asio::transfer_at_least(1), std::move(handler));
    }
    
    void TcpSslConnection::UpperLayerHandleConnect() {
        // SSL stuff.
        SslSocket.set_verify_mode(boost::asio::ssl::verify_peer);
        SslSocket.set_verify_callback(
                                      std::bind(&TcpSslConnection::VerifyCertificate, this,
                                                std::placeholders::_1, std::placeholders::_2));

        std::cout << "tcp connected" << std::endl;
        SslSocket.async_handshake(boost::asio::ssl::stream_base::client,
                                  std::bind(&TcpSslConnection::HandleHandshake, this,
                                            std::placeholders::_1));
    }

    
    bool TcpSslConnection::VerifyCertificate(bool preverified,
                           boost::asio::ssl::verify_context& ctx) {
        std::cout << "Verifying certificate, pre-verified: " << std::string(preverified ? "true" : "false");
        // FIX ME - verify.
        
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        std::cout << "Verifying, subject: " << subject_name << "\n";
        
        char issuer_name[256];
        X509_NAME_oneline(X509_get_issuer_name(cert), issuer_name, 256);
        std::cout << "Verifying, issuer: " << issuer_name << "\n";
        
        return preverified;
    }

    
    void TcpSslConnection::HandleHandshake(const boost::system::error_code ec) {
        if (ec)
        {
            _errorCallback(shared_from_this(), ec);
            return;
        }
        
        std::cout << "SSL handle handshake" << std::endl;
        _connectCallback(shared_from_this());

    }
    
}