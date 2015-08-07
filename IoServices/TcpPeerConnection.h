//
//  TcpPeerConnection.h
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__TcpPeerConnection__
#define __HelloAsio__TcpPeerConnection__

#include "IoBufferWrapper.h"
#include <memory>
#include <mutex>
#include <deque>
#include <boost/asio.hpp>


namespace HelloAsio {
    // FIX ME - this deserves to be a class.
    
    struct TcpPeerConnection;
    
    using WriteCompletionCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, boost::system::error_code)>;

    using ReadErrorCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, boost::system::error_code)>;

    using ReadSomeCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, std::size_t bytesRead)>;
    
    struct TcpPeerConnection : public std::enable_shared_from_this<TcpPeerConnection> {
        boost::asio::ip::tcp::socket PeerSocket;
        boost::asio::ip::tcp::endpoint PeerEndPoint;
        std::mutex Mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> OutQueue;
        const WriteCompletionCallback ErrorCallback;

        TcpPeerConnection(boost::asio::io_service* ioService, WriteCompletionCallback&& errorCallback);
        
        void AsyncWrite(std::string&& msg);
        
    private:
        
        void LaunchWrite();
        
        void WriteHandler(
                          std::shared_ptr<TcpPeerConnection> conn,
                          std::shared_ptr<IoBufferWrapper> bufWrapper,
                          boost::system::error_code ec,
                          std::size_t written);

    };
}

#endif /* defined(__HelloAsio__TcpPeerConnection__) */
