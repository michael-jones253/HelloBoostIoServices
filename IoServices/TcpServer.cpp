//
//  TcpServer.cpp
//  AsyncIo
//
//  Created by Michael Jones on 29/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#include "stdafx.h"
#include "TcpServer.h"

#include <iostream>
#include <thread>
#include <boost/bind.hpp>

namespace
{
	const int ChunkSize = 12;
}

namespace AsyncIo
{
	/// <summary>
	/// For async IO with client connections.
	/// NB this class is managed internally by the IO services and not to be instantiated by application code.
	/// </summary>
	/// <param name="ioService">The boost IO service.</param>
	/// <param name="port">The port to listen on.</param>
	/// <param name="readSomeCb">Client read some data callback.</param>
    TcpServer::TcpServer(boost::asio::io_service* ioService, int port, ReadSomeCallback&& readSomeCb) :
    _ioService{ioService},
	_mutex{},
    _port{port},
    _acceptor{},
    _peerConnections{},
    _readSomeCb{ std::move(readSomeCb) }
    {
    }
    
	TcpServer::TcpServer(TcpServer&& rhs) : _mutex{}
	{
        _ioService = rhs._ioService;
        _port = rhs._port;
        _acceptor = std::move(rhs._acceptor);
        _peerConnections = std::move(rhs._peerConnections);
        _readSomeCb = std::move(rhs._readSomeCb);
    }

	TcpServer& TcpServer::operator=(TcpServer&& rhs)
	{
		_ioService = rhs._ioService;
		_port = rhs._port;
		_acceptor = std::move(rhs._acceptor);
		_peerConnections = std::move(rhs._peerConnections);
		_readSomeCb = std::move(rhs._readSomeCb);

		return *this;
	}

    TcpServer::~TcpServer()
	{
		if (!_readSomeCb)
		{
			// We have been moved and there is no state to stop.
			return;
		}

        Stop();
    }
    
    void TcpServer::Start()
	{
        _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                                                             *_ioService,
                                                             boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _port));
        AsyncAccept();
    }
    
    void TcpServer::Stop()
	{
		if (!_acceptor)
		{
			return;
		}

        _acceptor->close();
        CloseAllPeerConnections();
    }
    
    void TcpServer::AcceptHandler(std::shared_ptr<TcpPeerConnection> acceptedConn, const boost::system::error_code& ec)
	{
        if (ec != 0)
		{
            std::cerr << "Accept error: " << ec << std::endl;
        }
        
        auto available = [this, acceptedConn](size_t available) {
            // std::cout << "Got stuff available: " << acceptedConn->PeerEndPoint << available << std::endl;
            _readSomeCb(acceptedConn, available);
        };
        
        acceptedConn->BeginChainedRead(std::move(available), ChunkSize);
        
		std::lock_guard<std::mutex> connGuard(_mutex);
        _peerConnections.push_back(acceptedConn);
        std::cout << "Accepted: " << _peerConnections.size() << std::endl;
        
        // Kick off another async accept to handle another connection.
        AsyncAccept();
    }
    
    void TcpServer::AsyncAccept()
	{
        auto errorHandler = std::bind(&TcpServer::ErrorHandler, this, std::placeholders::_1, std::placeholders::_2);
        
        auto conn = std::make_shared<TcpPeerConnection>(_ioService, std::move(errorHandler));

        auto acceptor = std::bind(&TcpServer::AcceptHandler, this, conn, std::placeholders::_1);

        // Async accept does not block and takes references to the socket and end point of the connection.
        // The connection smart pointer is kept alive by being bound to the acceptor callback.
        _acceptor->async_accept(conn->PeerSocket, conn->PeerEndPoint, std::move(acceptor));
    }
    
    void TcpServer::CloseAllPeerConnections()
	{
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto& conn : _peerConnections)
		{
            conn->PeerSocket.close();
        }
    }
    
    /// <summary>
    /// Client connection error handler to close the in error connection and remove it. This is called
    /// from the context of the boost IO service.
    /// </summary>
    /// <param name="conn">The TCP connection in error state.</param>
    /// <param name="ec">The boost error code.</param>
	void TcpServer::ErrorHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec)
	{
        if (ec != 0) {
			// FIX ME log this.
            std::cerr << "Write error" << std::endl;
            
            std::lock_guard<std::mutex> guard(_mutex);
            conn->PeerSocket.close();
            for (auto pos = _peerConnections.begin(); pos != _peerConnections.end(); pos++)
			{
                if (pos->get()->PeerSocket.is_open())
				{
                    continue;
                }
                
                std::cerr << "ERASING PEER: " << pos->get()->PeerEndPoint.address() << std::endl;
                _peerConnections.erase(pos);
                break;
            }
        }
    }

    void TcpServer::SendMessageToAllPeers(const std::string& msg, bool nullTerminate)
	{
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto& conn : _peerConnections)
		{

            // Allocate message for move.
            auto msgBuf = msg;
            conn->AsyncWrite(std::move(msgBuf), nullTerminate);
        }
    }
    
}
