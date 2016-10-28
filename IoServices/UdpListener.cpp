//
//  TcpConnection.cpp
//  AsyncIo
//
//  Created by Michael Jones on 29/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include "stdafx.h"

#include "UdpListener.h"
#include "IoLogConsumer.h"

#include <iostream>
#include <future>

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
		SetupCircularBufferCallbacks();
    }

	UdpListener::UdpListener(boost::asio::io_service* ioService, int port) :
		PeerSocket{ *ioService, udp::endpoint(udp::v4(), static_cast<unsigned short>(port)) },
		PeerEndPoint{},
		Mutex{},
		mOutQueue{},
		_errorCallback{},
		_connectCallback{},
		_asyncConnected{},
		_readBuffer{}
	{
		SetupCircularBufferCallbacks();
	}

	UdpListener::UdpListener(boost::asio::io_service* ioService) :
		PeerSocket{ *ioService },
		PeerEndPoint{},
		Mutex{},
		mOutQueue{},
		_errorCallback{},
		_connectCallback{},
		_asyncConnected{},
		_readBuffer{}
	{
		SetupCircularBufferCallbacks();
	}

	UdpListener::~UdpListener()
	{
		PeerSocket.close();
	}


	void UdpListener::SetupCircularBufferCallbacks() {
		auto cb = [this](uint8_t* bufPtr, size_t len, std::function<void(size_t)>&&handler) {
			auto myHandler = move(handler);
			auto boostHandler = [this, myHandler](const boost::system::error_code& ec,
				std::size_t bytes_transferred) {
				if (/* ec.value() != 10061 && */(bytes_transferred == 0 || ec != 0)) { // Weird windows/boost behavior: read with error code 10061 (connection refused) after write on loopback un-bound socket.
					auto me = this->shared_from_this();
					_readBuffer.EndReadSome();
					_errorCallback(me, ec);
					return;
				}

				myHandler(bytes_transferred);
			};

			// Unless buffer is created with a mutable pointer the boost buffer will not be mutable.
			auto boostBuf = boost::asio::buffer(const_cast<uint8_t*>(bufPtr), len);
			if (_asyncConnected)
			{
				PeerSocket.async_receive(boostBuf, std::move(boostHandler));
			}
			else
			{
				PeerSocket.async_receive_from(boostBuf, PeerEndPoint, std::move(boostHandler));
			}
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

	void UdpListener::JoinMulticastGroup(const boost::asio::ip::address& multicastAddr) {
		auto joinOption = boost::asio::ip::multicast::join_group(multicastAddr);
		PeerSocket.set_option(joinOption);
	}

    void UdpListener::EnableBroadcast()
    {
        if (!PeerSocket.is_open())
        {
            PeerSocket.open(udp::v4());
        }

        PeerSocket.set_option(boost::asio::socket_base::broadcast(true));
    }

	void UdpListener::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto bufWrapper = std::make_shared<UdpBufferWrapper>(std::move(msg), nullTerminate);

		QueueOrWriteBuffer(bufWrapper);
	}

	void UdpListener::AsyncSendTo(std::string&& msg, const std::string& destIp, int port, bool nullTerminate)
	{
		auto bufWrapper = std::make_shared<UdpBufferWrapper>(std::move(msg), destIp, port, nullTerminate);

		QueueOrWriteBuffer(bufWrapper);
	}

	void UdpListener::AsyncWrite(std::vector<uint8_t>&& msg)
	{
		auto bufWrapper = std::make_shared<UdpBufferWrapper>(move(msg));

		QueueOrWriteBuffer(bufWrapper);
	}
    
	void UdpListener::SetupChainedRead(IoNotifyAvailableCallback&& available, AsyncIo::UdpErrorCallback&& errCb, int datagramSize, bool immediate)
	{
		_errorCallback = std::move(errCb);
        _readBuffer.BeginChainedRead(std::move(available), datagramSize, immediate);
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
		if (mOutQueue.front()->DestEp() != nullptr)
		{
			if (!PeerSocket.is_open())
			{
				PeerSocket.open(udp::v4());
			}

            auto doBroadcast = mOutQueue.front()->DestEp()->IpAddress == "255.255.255.255";
            
            if (doBroadcast)
            {
                auto boostIp = boost::asio::ip::address_v4::broadcast();
                boost::asio::ip::udp::endpoint destEp(boostIp, mOutQueue.front()->DestEp()->Port);
                PeerSocket.async_send_to(mOutQueue.front()->ToBoost(), destEp, std::move(handler));
            }
            else
            {
                auto boostIp = boost::asio::ip::address::from_string(mOutQueue.front()->DestEp()->IpAddress);
                boost::asio::ip::udp::endpoint destEp(boostIp, mOutQueue.front()->DestEp()->Port);
                PeerSocket.async_send_to(mOutQueue.front()->ToBoost(), destEp, std::move(handler));
            }
		}
		else
		{
			// Connected socket.
			PeerSocket.async_send(mOutQueue.front()->ToBoost(), std::move(handler));
		}
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
    
	// For unbound unconnected listeners.
	void UdpListener::LaunchRead()
	{
		auto launchRead = [this]() {
			_readBuffer.BeginReadSome();
		};

		auto handle = std::async(std::launch::async, std::move(launchRead));
		handle.get();
	}

	void UdpListener::ConnectHandler(std::shared_ptr<UdpListener> conn, boost::system::error_code ec)
	{
		auto launchRead = [this]() {
			_readBuffer.BeginReadSome();
		};


		if (ec != 0) {
			_errorCallback(conn, ec);
		}
		else {
			// If this is a connect of an un-bound socket then launch the chained read.
			_asyncConnected.store(true);
			auto handle = std::async(std::launch::async, std::move(launchRead));
			handle.get();
			_connectCallback();
		}

	}

    void UdpListener::WriteHandler(
                                         std::shared_ptr<UdpListener> conn,
                                         std::shared_ptr<UdpBufferWrapper> bufWrapper,
                                         boost::system::error_code ec,
                                         std::size_t written)
	{        
        if (written != bufWrapper->BoostSize())
		{
            LOG() << "Incomplete write, buffer: " << bufWrapper->BoostSize() << " written: " << written << std::endl;
			// Boost method to stop an async read is to close the socket.
			// The async read callback will then call the error callback to cleanup the connection and inform the application.
			conn->PeerSocket.close();
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

	void UdpListener::QueueOrWriteBuffer(std::shared_ptr<UdpBufferWrapper> bufWrapper)
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
