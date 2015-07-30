//
//  TcpServer.h
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __HelloAsio__TcpServer__
#define __HelloAsio__TcpServer__

#include "TcpPeerConnection.h"

#include <boost/asio.hpp>
#include <memory>
#include <vector>

namespace HelloAsio {
    class TcpServer
    {
    private:
        boost::asio::io_service* _ioService;
        int _port;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
        std::vector<TcpPeerConnection> _peerConnections;
    public:
        TcpServer(boost::asio::io_service* ioService, int port);
        TcpServer(TcpServer&& rhs);
        ~TcpServer();
        void Start();
        void Stop();
    private:
        void AcceptHandler(std::shared_ptr<TcpPeerConnection> acceptedConn, const boost::system::error_code& ec);
        void AsyncAccept();
        void CloseAllPeerConnections();
    };
}
#endif /* defined(__HelloAsio__TcpServer__) */
