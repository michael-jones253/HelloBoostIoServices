//
//  TcpConnection.cpp
//  HelloAsio
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"

#include "TcpPeerConnection.h"
#include <iostream>

namespace HelloAsio
{
    TcpPeerConnection::TcpPeerConnection(boost::asio::io_service* ioService, const HelloAsio::ErrorCallback& errorCallback) :
        PeerSocket{*ioService},
        PeerEndPoint{},
        Mutex{},
        OutQueue{},
		_errorCallback{ std::move(errorCallback) },
        _readBuffer{} {
            auto cb = [this](uint8_t* bufPtr, size_t len, std::function<void(size_t)>&&handler) {
				auto myHandler = move(handler);
                auto boostHandler = [this, myHandler](const boost::system::error_code& ec,
                              std::size_t bytes_transferred) {
                    if (bytes_transferred == 0 || ec != 0) {
						auto me = this->shared_from_this();
						_readBuffer.EndReadSome();
						_errorCallback(me, ec);
                        return;
                    }
                    
                    myHandler(bytes_transferred);
                    
                };

                // Unless buffer is created with a mutable pointer the boost buffer will not be mutable.
                // FIX ME auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(_readBuffer.Get()), len);
				auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(bufPtr), len);
				boost::asio::async_read(PeerSocket, boostBuf, std::move(boostHandler));

            };
            
            IoCircularBuffer bufWithCb{ cb };
            _readBuffer = std::move(bufWithCb);
    }

    void TcpPeerConnection::AsyncWrite(std::string&& msg) {
        auto hack = std::move(msg);
        auto bufWrapper = std::make_shared<IoBufferWrapper>(hack);
        
        // Boost documentation says that for each stream only one async write can be outstanding at a time.
        // So we queue rather than launch straight away.
        std::lock_guard<std::mutex> guard(Mutex);
        OutQueue.push_back(bufWrapper);
        
        if (OutQueue.size() > 1) {
            // We have the lock, so if the queue has more than one, then a chained launch is guaranteed.
            return;
        }
        
        LaunchWrite();
    }

	void TcpPeerConnection::AsyncConnect(ConnectCallback&& connectCb, std::string ipAddress, int port)
	{
		boost::asio::ip::tcp::endpoint remoteEp{ boost::asio::ip::address::from_string(ipAddress), static_cast<unsigned short>(port) };
		_connectCallback = std::move(connectCb);

		auto boostHandler = std::bind(&TcpPeerConnection::ConnectHandler, this, shared_from_this(),std::placeholders::_1);
		PeerEndPoint = std::move(remoteEp);
		PeerSocket.async_connect(PeerEndPoint, std::move(boostHandler));
	}

    
	void TcpPeerConnection::BeginChainedRead(IoNotifyAvailableCallback&& available, int chunkSize) {
        _readBuffer.BeginChainedRead(std::move(available), chunkSize);
    }

    void TcpPeerConnection::LaunchWrite() {
        
        // Boost buffer does not hang on to data, so we bind a shared pointer to buffer wrapper to the callback,
        // to ensure that the buffer lasts the lifetime of the async completion.
        // MJ update - this isn't really needed now, except under a shutdown situation where the queue gets cleared
        // and there is an async operation outstanding.
        auto handler = std::bind(
                                 &TcpPeerConnection::WriteHandler,
                                 this,
                                 shared_from_this(),
                                 OutQueue.front(),
                                 std::placeholders::_1,
                                 std::placeholders::_2);
        
        boost::asio::async_write(PeerSocket, boost::asio::buffer(OutQueue.front()->Buffer), std::move(handler));
    }
    
    void TcpPeerConnection::CopyTo(std::vector<uint8_t>& dest, int len) {
        _readBuffer.CopyTo(dest, len);
    }
    
	void TcpPeerConnection::ConnectHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec)
	{
		if (ec)
		{
			_errorCallback(conn, ec);
			return;
		}

		_connectCallback(conn);
	}

    void TcpPeerConnection::WriteHandler(
                                         std::shared_ptr<TcpPeerConnection> conn,
                                         std::shared_ptr<IoBufferWrapper> bufWrapper,
                                         boost::system::error_code ec,
                                         std::size_t written) {
        
        if (written != bufWrapper->Buffer.size()) {
            std::cerr << "Incomplete write, buffer: " << bufWrapper->Buffer.size() << " written: " << written << std::endl;
            conn->PeerSocket.close();
            _errorCallback(conn, ec);
            return;
        }
        
        // Discard processed message.
        std::lock_guard<std::mutex> guard(Mutex);
        OutQueue.pop_front();
        
        // If there are no more queued messages then nothing more to do, otherwise chain another async write onto
        // the next message in the queue.
        if (OutQueue.size() == 0) {
            return;
        }
        
        // Chain another async write onto for the next message in the queue.
        LaunchWrite();
    }

}
