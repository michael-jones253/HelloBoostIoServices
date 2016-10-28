//
//  TcpConnection.cpp
//  AsyncIo
//
//  Created by Michael Jones on 17/10/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//
#include "stdafx.h"

#include "TcpDomainConnection.h"
#include "IoLogConsumer.h"
#include <iostream>

namespace AsyncIo
{
    TcpDomainConnection::TcpDomainConnection(boost::asio::io_service* ioService, DomainErrorCallback&& errorCallback) :
        DomainSocket{*ioService},
        DomainEndPoint{},
        _mutex{},
        mOutQueue{},
		_errorCallback{ std::move(errorCallback) },
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

                AsyncReadSome(bufPtr, len, std::move(boostHandler));

            };
            
            IoCircularBuffer bufWithCb{ cb };
            _readBuffer = std::move(bufWithCb);
    }

	int TcpDomainConnection::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto bufWrapper = std::make_shared<IoBufferWrapper>(move(msg), nullTerminate);

        // Boost documentation says that for each stream only one async write can be outstanding at a time.
        // So we queue rather than launch straight away.
        std::lock_guard<std::mutex> guard(_mutex);
        mOutQueue.push_back(bufWrapper);
        
        if (mOutQueue.size() > 1) {
            // We have the lock, so if the queue has more than one, then a chained launch is guaranteed.
			return static_cast<int>(mOutQueue.size());
		}
        
        LaunchWrite();
		return static_cast<int>(mOutQueue.size());
    }

	int TcpDomainConnection::AsyncWrite(std::vector<uint8_t>&& msg)
	{
		auto bufWrapper = std::make_shared<IoBufferWrapper>(move(msg));
		// Boost documentation says that for each stream only one async write can be outstanding at a time.
		// So we queue rather than launch straight away.
		std::lock_guard<std::mutex> guard(_mutex);
		mOutQueue.push_back(bufWrapper);

		if (mOutQueue.size() > 1) {
			// We have the lock, so if the queue has more than one, then a chained launch is guaranteed.
			return static_cast<int>(mOutQueue.size());
		}

		LaunchWrite();

		return static_cast<int>(mOutQueue.size());
	}


	void TcpDomainConnection::AsyncConnect(DomainConnectCallback&& connectCb, const std::string& name)
	{
		boost::asio::local::stream_protocol::endpoint remoteEp{ name };
		_connectCallback = std::move(connectCb);

		auto boostHandler = std::bind(&TcpDomainConnection::ConnectHandler, this, shared_from_this(),std::placeholders::_1);
		DomainEndPoint = std::move(remoteEp);
		DomainSocket.async_connect(DomainEndPoint, std::move(boostHandler));
	}

	void TcpDomainConnection::BeginChainedRead(
		IoNotifyAvailableCallback&& available,
		DomainErrorCallback&& errorCallback,
		int chunkSize)
	{
		_errorCallback = std::move(errorCallback);
		_readBuffer.BeginChainedRead(std::move(available), chunkSize);
	}

    void TcpDomainConnection::LaunchWrite() {
        
        // Boost buffer does not hang on to data, so we bind a shared pointer to buffer wrapper to the callback,
        // to ensure that the buffer lasts the lifetime of the async completion.
        // MJ update - this isn't really needed now, except under a shutdown situation where the queue gets cleared
        // and there is an async operation outstanding.
        auto handler = std::bind(
                                 &TcpDomainConnection::WriteHandler,
                                 this,
                                 shared_from_this(),
                                 mOutQueue.front(),
                                 std::placeholders::_1,
                                 std::placeholders::_2);
        
        boost::asio::async_write(DomainSocket, mOutQueue.front()->ToBoost(), std::move(handler));
	}
    
    void TcpDomainConnection::CopyTo(std::vector<uint8_t>& dest, int len)
	{
        _readBuffer.CopyTo(dest, len);
    }

	void TcpDomainConnection::Close()
	{
		DomainSocket.close();
	}
    
	void TcpDomainConnection::ConnectHandler(std::shared_ptr<TcpDomainConnection> conn, boost::system::error_code ec)
	{
		if (ec)
		{
			_errorCallback(conn, ec);
			return;
		}

		_connectCallback(shared_from_this());
	}

    void TcpDomainConnection::WriteHandler(
                                         std::shared_ptr<TcpDomainConnection> conn,
                                         std::shared_ptr<IoBufferWrapper> bufWrapper,
                                         boost::system::error_code ec,
                                         std::size_t written)
	{
        
        if (written != bufWrapper->BoostSize())
		{
			LOG() << "Incomplete write, buffer: " << bufWrapper->BoostSize() << " written: " << written << std::endl;
			// Boost method to stop an async read is to close the socket.
			// The async read callback will then call the error callback to cleanup the connection and inform the application.
			conn->DomainSocket.close();

            return;
        }
        
        // Discard processed message.
        std::lock_guard<std::mutex> guard(_mutex);
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

    void TcpDomainConnection::AsyncReadSome(uint8_t* bufPtr, size_t len, BoostIoHandler&& handler) {
        // Unless buffer is created with a mutable pointer the boost buffer will not be mutable.
        auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(bufPtr), len);
        boost::asio::async_read(DomainSocket, boostBuf, boost::asio::transfer_at_least(1), std::move(handler));
    }
}

