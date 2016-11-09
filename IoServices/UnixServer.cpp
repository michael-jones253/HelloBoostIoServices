//
//  UnixServer.cpp
//  AsyncIo
//
//  Created by Michael Jones on 29/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#include "stdafx.h"
#include "UnixServer.h"
#include "TcpDomainConnection.h"
#include "IoLogConsumer.h"

#include <cstdio>
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
    /// <param name="path">The path to listen on.</param>
    /// <param name="acceptsStream">Client connection accepted callback.</param>
    /// <param name="readSomeCb">Client read some data callback.</param>
    UnixServer::UnixServer(boost::asio::io_service* ioService, const std::string& path, UnixAcceptStreamCallback&& acceptsStream, UnixReadStreamCallback&& readSomeCb) :
    _ioService{ioService},
    _path{path},
    _acceptStreamCb{ std::move(acceptsStream) },
    _readSomeCb{ std::move(readSomeCb) }
    {
    }
    
    UnixServer::UnixServer(UnixServer&& rhs) : _mutex{}
    {
        _ioService = rhs._ioService;
        _path = rhs._path;
        _acceptor = std::move(rhs._acceptor);
        _domainConnections = std::move(rhs._domainConnections);
        _acceptStreamCb = std::move(rhs._acceptStreamCb);
        _readSomeCb = std::move(rhs._readSomeCb);
    }
    
    UnixServer& UnixServer::operator=(UnixServer&& rhs)
    {
        _ioService = rhs._ioService;
        _path = rhs._path;
        _acceptor = std::move(rhs._acceptor);
        _domainConnections = std::move(rhs._domainConnections);
        _acceptStreamCb = std::move(rhs._acceptStreamCb);
        _readSomeCb = std::move(rhs._readSomeCb);
        
        return *this;
    }
    
    UnixServer::~UnixServer()
    {
        if (!_readSomeCb)
        {
            // We have been moved and there is no state to stop.
            return;
        }
        
        Stop();
    }
    
    void UnixServer::Start()
    {
        std::remove(_path.c_str());

        _acceptor = std::make_unique<boost::asio::local::stream_protocol::acceptor>(
                                                                     *_ioService,
                                                                     boost::asio::local::stream_protocol::endpoint(_path));
        AsyncAccept();
    }
    
    
    void UnixServer::Stop()
    {
        if (!_acceptor)
        {
            return;
        }
        
        _acceptor->close();
        CloseAllUnixConnections();
    }
    
    void UnixServer::OnAccept(std::shared_ptr<TcpDomainConnection> acceptedConn) {
        auto streamConn = std::make_shared<UnixStreamConnection>(acceptedConn, false /* is server side */);
        auto available = [this, streamConn](size_t available) {
            // std::cout << "Got stuff available: " << acceptedConn->DomainEndPoint << available << std::endl;
            _readSomeCb(streamConn, static_cast<int>(available));
        };
        
        // Review: opportunity to have the error handler take a stream connection parameter.
        auto errorHandler = std::bind(&UnixServer::ErrorHandler, this, std::placeholders::_1, std::placeholders::_2);
        
        acceptedConn->BeginChainedRead(std::move(available), std::move(errorHandler), ChunkSize);
        
        std::lock_guard<std::mutex> connGuard(_mutex);
        _domainConnections.push_back(acceptedConn);
        LOG() << "Accepted: " << _domainConnections.size() << std::endl;
        
        // Notify the application that a client has connected.
        _acceptStreamCb(streamConn);
    }
    
    void UnixServer::AcceptHandler(std::shared_ptr<TcpDomainConnection> acceptedConn, const boost::system::error_code& ec)
    {
        if (ec != 0)
        {
            LOG() << "Accept error: " << ec << std::endl;
            // Review: call an error callback.
        }
        else {
            OnAccept(acceptedConn);
        }
        
        // Kick off another async accept to handle another connection.
        AsyncAccept();
    }
    
    void UnixServer::AsyncAccept()
    {
        auto errorHandler = std::bind(&UnixServer::ErrorHandler, this, std::placeholders::_1, std::placeholders::_2);
        
        auto conn = std::make_shared<TcpDomainConnection>(_ioService, std::move(errorHandler));
        
        auto acceptor = std::bind(&UnixServer::AcceptHandler, this, conn, std::placeholders::_1);
        
        // Async accept does not block and takes references to the socket and end point of the connection.
        // The connection smart pointer is kept alive by being bound to the acceptor callback.
        _acceptor->async_accept(conn->DomainSocket, conn->DomainEndPoint, std::move(acceptor));
    }
    
    void UnixServer::CloseAllUnixConnections()
    {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto& conn : _domainConnections)
        {
            conn->DomainSocket.close();
        }
    }
    
    /// <summary>
    /// Client connection error handler to close the in error connection and remove it. This is called
    /// from the context of the boost IO service.
    /// </summary>
    /// <param name="conn">The TCP connection in error state.</param>
    /// <param name="ec">The boost error code.</param>
    void UnixServer::ErrorHandler(std::shared_ptr<TcpDomainConnection> conn, boost::system::error_code ec)
    {
        if (ec != 0) {
            std::lock_guard<std::mutex> guard(_mutex);
            conn->DomainSocket.close();
            
            auto SocketClosedPredicate = [](std::shared_ptr<TcpDomainConnection> conn) {
                auto shouldErase = !(conn->DomainSocket.is_open());
                if (shouldErase)
                {
					LOG() << "Unix server ERASING peer: " << conn->DomainEndPoint.path() << std::endl;
                }
                
                return shouldErase;
            };
            
            _domainConnections.erase(std::remove_if(_domainConnections.begin(), _domainConnections.end(), SocketClosedPredicate), _domainConnections.end());
        }
    }
    
    void UnixServer::SendMessageToAllClients(const std::string& msg, bool nullTerminate)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto& conn : _domainConnections)
        {
            
            // Allocate message for move.
            auto msgBuf = msg;
            conn->AsyncWrite(std::move(msgBuf), nullTerminate);
        }
    }
    
}
