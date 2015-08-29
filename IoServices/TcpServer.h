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
        TcpServer(boost::asio::io_service* ioService, int port, ReadSomeCallback&& readSomeCb);
        TcpServer(TcpServer&& rhs);
        ~TcpServer();
        void Start();
        void Stop();
        void SendMessageToAllPeersDeprecated(const std::string& msg);
        void SendMessageToAllPeers(const std::string& msg);
        int GetPort() const { return _port; }
    private:
        void AcceptHandler(std::shared_ptr<TcpPeerConnection> acceptedConn, const boost::system::error_code& ec);
        void AsyncAccept();
        void WriteHandlerDeprecated(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec, std::size_t written);
        void WriteHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec);
        void CloseAllPeerConnections();
    };
}
#endif /* defined(__HelloAsio__TcpServer__) */
