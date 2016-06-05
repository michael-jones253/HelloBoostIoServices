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
#include <boost/system/error_code.hpp>
#include "TcpPeerConnection.h"
#include <boost/asio/ssl.hpp>

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif


namespace AsyncIo {
    
    class TcpSslConnection;
    
    class TcpSslConnection final : public TcpPeerConnection {
    private:
        
    public:
        boost::asio::ssl::context Ctx;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> SslSocket;
        
        TcpSslConnection(boost::asio::io_service* ioService,
                         boost::asio::ssl::context&& ctx,
                         AsyncIo::ErrorCallback&& errorCallback);
        
        ~TcpSslConnection() {
            std::cout << "Closing TCP SSL connection: " << PeerEndPoint << std::endl;
        }
        
    private:
        void AsyncWriteToSocket(std::shared_ptr<IoBufferWrapper>& bufferWrapper, BoostIoHandler&& handler) override;
        void AsyncReadSome(uint8_t* bufPtr, size_t len, BoostIoHandler&& handler) override;
        void UpperLayerHandleConnect() override;

        bool VerifyCertificate(bool preverified,
                               boost::asio::ssl::verify_context& ctx);
        
        void LaunchWrite();
        
        void ConnectHandler(std::shared_ptr<TcpSslConnection> conn, boost::system::error_code ec);
        
        void HandleHandshake(const boost::system::error_code error);
        
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
