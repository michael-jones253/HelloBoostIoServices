//
//  TcpConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "TcpPeerConnection.h"
#include <iostream>

namespace HelloAsio
{
    TcpPeerConnection::TcpPeerConnection(boost::asio::io_service* ioService) :
        PeerSocket{*ioService},
        PeerEndPoint{} {
        
    }
    
    void TcpPeerConnection::WriteHandler(
                                         const WriteCompletionCallback& serverCallback,
                                         std::shared_ptr<TcpPeerConnection> conn,
                                         std::shared_ptr<IoBufferWrapper> bufWrapper,
                                         boost::system::error_code ec,
                                         std::size_t written) {
        if (written != bufWrapper->Buffer.size()) {
            std::cerr << "Incomplete write, buffer: " << bufWrapper->Buffer.size() << " written: " << written << std::endl;
            conn->PeerSocket.close();
            serverCallback(conn, ec);
            return;
        }
        
        {
            std::lock_guard<std::mutex> guard(Mutex);
            OutQueue.pop_front();
            
            // If there are no more queued messages then nothing more to do, otherwise chain another async write onto
            // the next message in the queue.
            if (OutQueue.size() == 0) {
                return;
            }

            // Boost buffer does not hang on to data, so we bind a shared pointer to buffer wrapper to the callback,
            // to ensure that the buffer lasts the lifetime of the async completion.
            auto handler = std::bind(
                                     &TcpPeerConnection::WriteHandler,
                                     this,
                                     serverCallback,
                                     shared_from_this(),
                                     OutQueue.front(),
                                     std::placeholders::_1,
                                     std::placeholders::_2);
            
            boost::asio::async_write(PeerSocket, boost::asio::buffer(OutQueue.front()->Buffer), std::move(handler));
        }        
    }

    // FIX ME - probably don't want to move serverCallback - const ref it.
    void TcpPeerConnection::AsyncWrite(std::string&& msg, WriteCompletionCallback&& serverCallback) {
        auto hack = std::move(msg);
        auto bufWrapper = std::make_shared<IoBufferWrapper>(hack);
        
        {
            std::lock_guard<std::mutex> guard(Mutex);
            OutQueue.push_back(bufWrapper);
        }
        
        // Boost documentation says that for each stream only one async write can be outstanding at a time.
        if (OutQueue.size() > 1) {
            return;
        }

        // Boost buffer does not hang on to data, so we bind a shared pointer to buffer wrapper to the callback,
        // to ensure that the buffer lasts the lifetime of the async completion.
        auto handler = std::bind(
                                 &TcpPeerConnection::WriteHandler,
                                 this,
                                 std::move(serverCallback),
                                 shared_from_this(),
                                 bufWrapper,
                                 std::placeholders::_1,
                                 std::placeholders::_2);
        
        
        boost::asio::async_write(PeerSocket, boost::asio::buffer(bufWrapper->Buffer), std::move(handler));
        
    }

}
