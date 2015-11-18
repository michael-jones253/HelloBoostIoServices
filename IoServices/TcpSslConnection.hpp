//
//  TcpSslConnection.hpp
//  HelloAsio
//
//  Created by Michael Jones on 14/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

#ifndef TcpSslConnection_hpp
#define TcpSslConnection_hpp

// FIX ME - probably need a later version of boost.
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "IoBufferWrapper.h"
#include "IoCircularBuffer.h"
#include <memory>
#include <mutex>
#include <functional>
#include <deque>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <vector>

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif


namespace AsyncIo {
    
    class TcpSslConnection;
    
    // using ErrorCallback = std::function<void(std::shared_ptr<TcpSslConnection>, boost::system::error_code)>;
    using ErrorCallback = std::function<void(std::shared_ptr<TcpSslConnection>, const boost::system::error_code&)>;
    
    using ReadSomeCallback = std::function<void(std::shared_ptr<TcpSslConnection>, std::size_t bytesAvailable)>;
    
    using SslConnectCallback = std::function<void(std::shared_ptr<TcpSslConnection>)>;
    
    class TcpSslConnection final : public std::enable_shared_from_this<TcpSslConnection> {
    private:
        std::mutex _mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> mOutQueue;
        ErrorCallback _errorCallback;
        SslConnectCallback _connectCallback;
        IoCircularBuffer _readBuffer;
        
    public:
        boost::asio::ip::tcp::socket PeerSocket;
        boost::asio::ip::tcp::endpoint PeerEndPoint;
        boost::asio::ssl::context Ctx;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> SslSocket;
        
        TcpSslConnection(boost::asio::io_service* ioService,
                         boost::asio::ssl::context&& ctx,
                         AsyncIo::ErrorCallback&& errorCallback);
        
        ~TcpSslConnection() {
            std::cout << "Closing TCP peer connection: " << PeerEndPoint << std::endl;
        }
        
        // NB returns backlog.
        int AsyncWrite(std::string&& msg, bool nullTerminate);
        
        // NB returns backlog.
        int AsyncWrite(std::vector<uint8_t>&& msg);
        
        void AsyncConnect(SslConnectCallback&& connectCb, std::string ipAddress, int port);
        
        void BeginChainedRead(IoNotifyAvailableCallback&& available,
                              AsyncIo::ErrorCallback&& errorCallback,
                              int chunkSize);
        
        boost::asio::ip::tcp::socket& GetPeerSocket() { return PeerSocket; }
        
        const uint8_t* Data() const { return _readBuffer.Get(); }
        
        size_t Size() const { return _readBuffer.Size(); }
        
        void Consume(size_t len) { _readBuffer.Consume(len); }
        
        void CopyTo(std::vector<uint8_t>& dest, int len);
        
    private:
        
        bool VerifyCertificate(bool preverified,
                               boost::asio::ssl::verify_context& ctx);
        
        void LaunchWrite();
        
        void ConnectHandler(std::shared_ptr<TcpSslConnection> conn, boost::system::error_code ec);
        
        void HandleHandshake(const boost::system::error_code& error);
        
        void WriteHandler(
                          std::shared_ptr<TcpSslConnection> conn,
                          std::shared_ptr<IoBufferWrapper> bufWrapper,
                          boost::system::error_code ec,
                          std::size_t written);
        
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif


#endif /* TcpSslConnection_hpp */
