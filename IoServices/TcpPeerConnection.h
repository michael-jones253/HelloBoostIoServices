//
//  TcpPeerConnection.h
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__TcpPeerConnection__
#define __HelloAsio__TcpPeerConnection__

// FIX ME - probably need a later version of boost.
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "IoBufferWrapper.h"
#include "IoCircularBuffer.h"
#include <memory>
#include <mutex>
#include <functional>
#include <deque>
#include <boost/asio.hpp>
#include <vector>

/* The classes below are exported */
#pragma GCC visibility push(default)
namespace HelloAsio {
    
    class TcpPeerConnection;
    
    // using ErrorCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, boost::system::error_code)>;
	using ErrorCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, const boost::system::error_code&)>;

    using ReadSomeCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, std::size_t bytesAvailable)>;

	using ConnectCallback = std::function<void(std::shared_ptr<TcpPeerConnection>)>;

    class TcpPeerConnection final : public std::enable_shared_from_this<TcpPeerConnection> {
    private:
        std::mutex Mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> OutQueue;
        const ErrorCallback _errorCallback;
		ConnectCallback _connectCallback;
        IoCircularBuffer _readBuffer;
        
    public:
        boost::asio::ip::tcp::socket PeerSocket;
        boost::asio::ip::tcp::endpoint PeerEndPoint;

		TcpPeerConnection(boost::asio::io_service* ioService, const HelloAsio::ErrorCallback& errorCallback);
		~TcpPeerConnection() {
			std::cout << "Closing TCP peer connection: " << PeerEndPoint << std::endl;
		}
        
        void AsyncWrite(std::string&& msg);

		void AsyncConnect(ConnectCallback&& connectCb, std::string ipAddress, int port);
        
        void BeginChainedRead(IoNotifyAvailableCallback&& available, int chunkSize);
        
        boost::asio::ip::tcp::socket& GetPeerSocket() { return PeerSocket; }
        
        const uint8_t* Data() const { return _readBuffer.Get(); }
        
        size_t Size() const { return _readBuffer.Size(); }
        
        void Consume(size_t len) { _readBuffer.Consume(len); }
        
        void CopyTo(std::vector<uint8_t>& dest, int len);
        
    private:
        
        void LaunchWrite();
        
		void ConnectHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec);

		void WriteHandler(
                          std::shared_ptr<TcpPeerConnection> conn,
                          std::shared_ptr<IoBufferWrapper> bufWrapper,
                          boost::system::error_code ec,
                          std::size_t written);

    };
}
#pragma GCC visibility pop

#endif /* defined(__HelloAsio__TcpPeerConnection__) */
