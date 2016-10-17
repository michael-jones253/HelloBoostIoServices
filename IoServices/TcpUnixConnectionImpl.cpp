//
//  TcpConnection.cpp
//  AsyncIo
//
//  Created by Michael Jones on 17/10/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//
#include "stdafx.h"

#include "TcpUnixConnectionImpl.h"
#include "IoLogConsumer.h"
#include <iostream>

namespace AsyncIo
{
    TcpUnixConnectionImpl::TcpUnixConnectionImpl(boost::asio::io_service* ioService, AsyncIo::ErrorCallback&& errorCallback) :
        UnixSocket{*ioService},
        UnixEndPoint{},
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

                // Virtual
                AsyncReadSome(bufPtr, len, std::move(boostHandler));

            };
            
            IoCircularBuffer bufWithCb{ cb };
            _readBuffer = std::move(bufWithCb);
    }

	int TcpUnixConnectionImpl::AsyncWrite(std::string&& msg, bool nullTerminate)
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

	int TcpUnixConnectionImpl::AsyncWrite(std::vector<uint8_t>&& msg)
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


	void TcpUnixConnectionImpl::AsyncConnect(ConnectCallback&& connectCb, const std::string& name)
	{
		boost::asio::local::stream_protocol::endpoint remoteEp{ name };
		_connectCallback = std::move(connectCb);

		auto boostHandler = std::bind(&TcpUnixConnectionImpl::ConnectHandler, this, shared_from_this(),std::placeholders::_1);
		UnixEndPoint = std::move(remoteEp);
		UnixSocket.async_connect(UnixEndPoint, std::move(boostHandler));
	}

	void TcpUnixConnectionImpl::BeginChainedRead(
		IoNotifyAvailableCallback&& available,
		AsyncIo::ErrorCallback&& errorCallback,
		int chunkSize)
	{
		_errorCallback = std::move(errorCallback);
		_readBuffer.BeginChainedRead(std::move(available), chunkSize);
	}

    void TcpUnixConnectionImpl::LaunchWrite() {
        
        // Boost buffer does not hang on to data, so we bind a shared pointer to buffer wrapper to the callback,
        // to ensure that the buffer lasts the lifetime of the async completion.
        // MJ update - this isn't really needed now, except under a shutdown situation where the queue gets cleared
        // and there is an async operation outstanding.
        auto handler = std::bind(
                                 &TcpUnixConnectionImpl::WriteHandler,
                                 this,
                                 shared_from_this(),
                                 mOutQueue.front(),
                                 std::placeholders::_1,
                                 std::placeholders::_2);
        
        // Virtual
        AsyncWriteToSocket(mOutQueue.front(), std::move(handler));
	}
    
    void TcpUnixConnectionImpl::CopyTo(std::vector<uint8_t>& dest, int len)
	{
        _readBuffer.CopyTo(dest, len);
    }

	void TcpUnixConnectionImpl::Close()
	{
		UnixSocket.close();
	}
    
	void TcpUnixConnectionImpl::ConnectHandler(std::shared_ptr<TcpUnixConnectionImpl> conn, boost::system::error_code ec)
	{
		if (ec)
		{
			_errorCallback(conn, ec);
			return;
		}

		UpperLayerHandleConnect();
	}

    void TcpUnixConnectionImpl::WriteHandler(
                                         std::shared_ptr<TcpUnixConnectionImpl> conn,
                                         std::shared_ptr<IoBufferWrapper> bufWrapper,
                                         boost::system::error_code ec,
                                         std::size_t written)
	{
        
        if (written != bufWrapper->BoostSize())
		{
			LOG() << "Incomplete write, buffer: " << bufWrapper->BoostSize() << " written: " << written << std::endl;
			// Boost method to stop an async read is to close the socket.
			// The async read callback will then call the error callback to cleanup the connection and inform the application.
			conn->UnixSocket.close();

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

}
