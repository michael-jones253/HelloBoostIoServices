//
//  TcpConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 5/04/2016.
//  Copyright © 2016 Michael Jones. All rights reserved.
//

#include "TcpConnection.hpp"
namespace AsyncIo {
    
    TcpConnection::TcpConnection(boost::asio::io_service* ioService, AsyncIo::ErrorCallback&& errorCallback) :
    TcpPeerConnection(ioService, move(errorCallback))
    {
    }
    
    void TcpConnection::AsyncWriteToSocket(std::shared_ptr<IoBufferWrapper>& bufferWrapper, BoostIoHandler&& handler) {
        boost::asio::async_write(PeerSocket, bufferWrapper->ToBoost(), std::move(handler));
        
    }
    
    void TcpConnection::AsyncReadSome(uint8_t* bufPtr, size_t len, BoostIoHandler&& handler) {
        // Unless buffer is created with a mutable pointer the boost buffer will not be mutable.
        auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(bufPtr), len);
        boost::asio::async_read(PeerSocket, boostBuf, boost::asio::transfer_at_least(1), std::move(handler));
    }
    
    void TcpConnection::UpperLayerHandleConnect() {
        _connectCallback(shared_from_this());
    }
}