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
        auto cb = [this](uint8_t* bufPtr, size_t len, std::function<void(size_t)>&&handler) {
            auto myHandler = move(handler);
            auto boostHandler = [this, myHandler](const boost::system::error_code& ec,
                                                  std::size_t bytes_transferred) {
                if (bytes_transferred == 0 || ec != 0) {
                    auto me = this->shared_from_this();
                    _readBuffer.EndReadSome();
                    _errorCallback(me, ec);
                    return;
                }
                
                myHandler(bytes_transferred);
                
            };
            
            // Unless buffer is created with a mutable pointer the boost buffer will not be mutable.
            auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(bufPtr), len);
            boost::asio::async_read(SslSocket, boostBuf, boost::asio::transfer_at_least(1), std::move(boostHandler));
            
        };
        
        IoCircularBuffer bufWithCb{ cb };
        _readBuffer = std::move(bufWithCb);
    
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
    
    void TcpSslConnection::BeginChainedRead(
                                             IoNotifyAvailableCallback&& available,
                                             AsyncIo::ErrorCallback&& errorCallback,
                                             int chunkSize)
    {
        _errorCallback = std::move(errorCallback);
        _readBuffer.BeginChainedRead(std::move(available), chunkSize);
    }

    void TcpSslConnection::ConnectHandler(std::shared_ptr<TcpSslConnection> conn, boost::system::error_code ec) {
        if (ec)
        {
            _errorCallback(conn, ec);
            return;
        }
        
        std::cout << "tcp connected" << std::endl;
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

    
    void TcpSslConnection::HandleHandshake(const boost::system::error_code ec) {
        if (ec)
        {
            _errorCallback(shared_from_this(), ec);
            return;
        }
        
        std::cout << "SSL handle handshake" << std::endl;
        _connectCallback(shared_from_this());

    }
    
    int TcpSslConnection::AsyncWrite(std::string&& msg, bool nullTerminate) {
        auto bufWrapper = std::make_shared<IoBufferWrapper>(move(msg), nullTerminate);
        
        // Boost documentation says that for each stream only one async write can be outstanding at a time.
        // So we queue rather than launch straight away.
        std::lock_guard<std::mutex> guard(_mutex);
        mOutQueue.push_back(bufWrapper);
        
        if (mOutQueue.size() > 1) {
            // We have the lock, so if the queue has more than one, then a chained launch is guaranteed.
            return static_cast<int>(mOutQueue.size());
        }
        
        LaunchWrite();
        return static_cast<int>(mOutQueue.size());
    }
    
    void TcpSslConnection::LaunchWrite() {
        
        // Boost buffer does not hang on to data, so we bind a shared pointer to buffer wrapper to the callback,
        // to ensure that the buffer lasts the lifetime of the async completion.
        // MJ update - this isn't really needed now, except under a shutdown situation where the queue gets cleared
        // and there is an async operation outstanding.
        auto handler = std::bind(
                                 &TcpSslConnection::WriteHandler,
                                 this,
                                 shared_from_this(),
                                 mOutQueue.front(),
                                 std::placeholders::_1,
                                 std::placeholders::_2);
        
        // boost::asio::async_write(PeerSocket, boost::asio::buffer(mOutQueue.front()->Buffer), std::move(handler));
        boost::asio::async_write(SslSocket, mOutQueue.front()->ToBoost(), std::move(handler));
    }

    void TcpSslConnection::WriteHandler(
                                         std::shared_ptr<TcpSslConnection> conn,
                                         std::shared_ptr<IoBufferWrapper> bufWrapper,
                                         boost::system::error_code ec,
                                         std::size_t written)
    {
        
        if (written != bufWrapper->BoostSize())
        {
            std::cerr << "Incomplete write, buffer: " << bufWrapper->BoostSize() << " written: " << written << std::endl;
            conn->PeerSocket.close();
            _errorCallback(conn, ec);
            return;
        }
        
        // Discard processed message.
        std::lock_guard<std::mutex> guard(_mutex);
        mOutQueue.pop_front();
        
        // If there are no more queued messages then nothing more to do, otherwise chain another async write onto
        // the next message in the queue.
        if (mOutQueue.size() == 0)
        {
            return;
        }
        
        // Chain another async write onto for the next message in the queue.
        LaunchWrite();
    }

}