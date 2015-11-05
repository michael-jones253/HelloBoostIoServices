//
//  TcpConnection.cpp
//  AsyncIo
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"

#include "UdpListener.h"
#include <iostream>

using namespace boost::asio::ip;

namespace AsyncIo
{
	UdpListener::UdpListener(boost::asio::io_service* ioService, const boost::asio::ip::address& address, int port) :
		PeerSocket{ *ioService, udp::endpoint(address, static_cast<unsigned short>(port)) },
        PeerEndPoint{},
        Mutex{},
        mOutQueue{},
		_errorCallback{},
		_connectCallback{},
        _readBuffer{}
	{
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
				auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(bufPtr), len);
				PeerSocket.async_receive_from(boostBuf, PeerEndPoint, std::move(boostHandler));
            };
            
            IoCircularBuffer bufWithCb{ cb };
            _readBuffer = std::move(bufWithCb);
    }

	// For already bound listeners.
	void UdpListener::AsyncConnect(const boost::asio::ip::address& destIp, int port, std::function<void()>&& connectHandler) {
		assert(_errorCallback != nullptr);
		_connectCallback = move(connectHandler);

		auto handler = std::bind(
			&UdpListener::ConnectHandler,
			this,
			shared_from_this(),
			std::placeholders::_1);
		
		PeerSocket.async_connect(udp::endpoint(destIp, static_cast<unsigned short>(port)), handler);
	}

	void UdpListener::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto bufWrapper = std::make_shared<IoBufferWrapper>(std::move(msg), nullTerminate);

		QueueOrWriteBuffer(bufWrapper);
	}

	void UdpListener::AsyncWrite(std::vector<uint8_t>&& msg)
	{
		auto bufWrapper = std::make_shared<IoBufferWrapper>(move(msg));

		QueueOrWriteBuffer(bufWrapper);
	}
    
	void UdpListener::BeginChainedRead(IoNotifyAvailableCallback&& available, AsyncIo::UdpErrorCallback&& errCb, int datagramSize)
	{
		_errorCallback = std::move(errCb);
        _readBuffer.BeginChainedRead(std::move(available), datagramSize);
    }

    void UdpListener::LaunchWrite()
	{
        
        // Boost buffer does not hang on to data, so we bind a shared pointer to buffer wrapper to the callback,
        // to ensure that the buffer lasts the lifetime of the async completion.
        // MJ update - this isn't really needed now, except under a shutdown situation where the queue gets cleared
        // and there is an async operation outstanding.
        auto handler = std::bind(
                                 &UdpListener::WriteHandler,
                                 this,
                                 shared_from_this(),
                                 mOutQueue.front(),
                                 std::placeholders::_1,
                                 std::placeholders::_2);
        
       PeerSocket.async_send(mOutQueue.front()->ToBoost(), std::move(handler));
	}
    
    void UdpListener::CopyTo(std::vector<uint8_t>& dest, int len)
	{
        _readBuffer.CopyTo(dest, len);
    }

	void UdpListener::StopListening()
	{
		// FIX ME if we close the socket, the error handler is called and removes the listener. This is good, but there may be thread contention for the listener map.

		//_readBuffer.EndReadSome();
		PeerSocket.close();
	}
    
	// For already bound listeners.
	void UdpListener::ConnectHandler(std::shared_ptr<UdpListener> conn, boost::system::error_code ec)
	{
		if (ec != 0) {
			_errorCallback(conn, ec);
		}
		else {
			_connectCallback();
		}
	}

    void UdpListener::WriteHandler(
                                         std::shared_ptr<UdpListener> conn,
                                         std::shared_ptr<IoBufferWrapper> bufWrapper,
                                         boost::system::error_code ec,
                                         std::size_t written)
	{        
        if (written != bufWrapper->BoostSize())
		{
            std::cerr << "Incomplete write, buffer: " << bufWrapper->BoostSize() << " written: " << written << std::endl;
            conn->PeerSocket.close();
            _errorCallback(conn, ec);
            return;
        }
        
        // Discard processed message.
        std::lock_guard<std::mutex> guard(Mutex);
        mOutQueue.pop_front();
        
        // If there are no more queued messages then nothing more to do, otherwise chain another async write onto
        // the next message in the queue.
        if (mOutQueue.size() == 0)
		{
            return;
        }
        
        // Chain another async write onto for the next message in the queue.
        LaunchWrite();
    }

	void UdpListener::QueueOrWriteBuffer(std::shared_ptr<IoBufferWrapper> bufWrapper)
	{
		// Boost documentation says that for each stream only one async write can be outstanding at a time.
		// So we queue rather than launch straight away.
		std::lock_guard<std::mutex> guard(Mutex);
		mOutQueue.push_back(bufWrapper);

		if (mOutQueue.size() > 1) {
			// We have the lock, so if the queue has more than one, then a chained launch is guaranteed.
			return;
		}

		LaunchWrite();
	}

}
