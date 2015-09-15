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
#include <mutex>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace HelloAsio {
    class TcpServer final {
    private:
        boost::asio::io_service* _ioService;
        std::mutex _mutex;
        int _port;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
        std::vector<std::shared_ptr<TcpPeerConnection>> _peerConnections;
        ReadSomeCallback _readSomeCb;
    public:
		TcpServer() = delete;
        TcpServer(boost::asio::io_service* ioService, int port, ReadSomeCallback&& readSomeCb);
        TcpServer(TcpServer&& rhs);
		TcpServer& operator=(TcpServer&& rhs);
		TcpServer(const TcpServer&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;

		~TcpServer();
        void Start();
        void Stop();
        void SendMessageToAllPeers(const std::string& msg, bool nullTerminate);
        int GetPort() const { return _port; }
    private:
        void AcceptHandler(std::shared_ptr<TcpPeerConnection> acceptedConn, const boost::system::error_code& ec);
        void AsyncAccept();
        void ErrorHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec);
        void CloseAllPeerConnections();
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__TcpServer__) */
