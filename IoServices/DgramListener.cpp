//
//  DgramListener.cpp
//  AsyncIo
//
//  Created by Michael Jones on 29/08/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//
#include "stdafx.h"

#include "DgramListener.h"
#include "UdpListener.h"
#include <sstream>

using namespace std;

namespace AsyncIo {

	DgramListener::DgramListener(std::shared_ptr<UdpListener> udpListener) :
		_udpListener{ udpListener }
	{

	}

	DgramListener::~DgramListener()
	{
		try
		{
			StopListening();
		}
		catch (std::exception& ex)
		{
			cout << "Error closing Datagram listener: " << ex.what() << endl;
		}
	}

	/// <summary>
	/// Only needed if writing. Once connected to the destination address all async writes go to this address.
	/// </summary>
	/// <param name="connectCb">The connect callback - if unsuccesful the error callback supplied for the bind will be called.</param>
	/// <param name="destIp">The destination ip in dot notation form.</param>
	/// <param name="port">Int the destination UDP port.</param>
	void DgramListener::AsyncConnect(DgramConnectCallback&& connectCb, const std::string& destIp, int port)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Async Connect Connection expired.");
		}

		auto connectCbCopy = move(connectCb);
		auto handler = [this, connectCbCopy]() {
			connectCbCopy(shared_from_this());
		};

		auto boostIp = boost::asio::ip::address::from_string(destIp);
		listener->AsyncConnect(boostIp, port, move(handler));
	}

	/// <summary>
	/// Allows an already bound Listener to join a multicast group for receiving multicast messages.
	/// </summary>
	/// <param name="multicastAddr">The multicast IP address in dot notation.</param>
	void DgramListener::JoinMulticastGroup(const std::string& multicastAddr)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Join Multi Cast Connection expired.");
		}

		auto boostIp = boost::asio::ip::address::from_string(multicastAddr);
		listener->JoinMulticastGroup(boostIp);
	}

    /// <summary>
    /// Allows an already bound Listener to send on the broadcast address.
    /// </summary>
    void DgramListener::EnableBroadcast()
    {
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Enable Broadcast Connection expired.");
		}

        listener->EnableBroadcast();
    }

	/// <summary>
	/// Asynchronous write of a string message.
	/// NB if the length of the message is greater than MTU it will be split across datagrams.
	/// </summary>
	/// <param name="msg">The string message to send.</param>
	/// <param name="nullTerminate">Whether to null terminate the string or not.</param>
	void DgramListener::AsyncWrite(std::string&& msg, bool nullTerminate)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Async Write Connection expired.");
		}

		listener->AsyncWrite(std::move(msg), nullTerminate);
	}

	/// <summary>
	/// Datagram send for unconnected socket.
	/// </summary>
	/// <param name="msg">The message to send.</param>
	/// <param name="destIp">The destination IP address in dot format.</param>
	/// <param name="port">The destination port.</param>
	/// <param name="nullTerminate">Whether to null terminate the string or not.</param>
	void DgramListener::AsyncSendTo(std::string&& msg, const std::string& destIp, int port, bool nullTerminate)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Async Send To Connection expired.");
		}

		listener->AsyncSendTo(std::move(msg), destIp, port, nullTerminate);
	}

	/// <summary>
	/// Asynchronous write of a string message.
	/// NB if the length of the message is greater than MTU it will be split across datagrams.
	/// </summary>
	/// <param name="msg">The byte vector message to send.</param>
	void DgramListener::AsyncWrite(std::vector<uint8_t>&& msg)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Async Write Connection expired.");
		}

		listener->AsyncWrite(std::move(msg));
	}

	/// <summary>
	/// Consume from the start of the circular buffer and copy to the destination buffer.
	/// </summary>
	/// <param name="buf">The destination buffer.</param>
	/// <param name="len">The length to be copied/extracted.</param>
	void DgramListener::ConsumeInto(uint8_t* buf, int len)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Consume Into Connection expired.");
		}

		memcpy(buf, listener->Data(), len);
		listener->Consume(len);
	}

	/// <summary>
	/// Consume from the start of the circular buffer and copy to the back of the destination.
	/// </summary>
	/// <param name="dest">The destination vector.</param>
	/// <param name="len">Length to be consumed/copied.</param>
	void DgramListener::ConsumeTo(std::vector<uint8_t>& dest, int len)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Consume To Connection expired.");
		}

		assert(len <= static_cast<int>(listener->Size()) && "Must extract within limit of available.");
		listener->CopyTo(dest, len);
		listener->Consume(len);
	}

	/// /// <summary>
	/// Returns a pointer to the start of circular buffer data.
	/// </summary>
	/// <returns>Pointer to received data.</returns>
	const uint8_t* DgramListener::Data() const
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Data Connection expired.");
		}

		return listener->Data();
	}

	/// <summary>
	/// Returns the size of the available circular buffer data.
	/// </summary>
	/// <returns>Size of the received data.</returns>
	size_t DgramListener::Size() const
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Size Connection expired.");
		}

		return listener->Size();
	}

	void DgramListener::Consume(int len)
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Consume Connection expired.");
		}

		listener->Consume(len);
	}

	/// <summary>
	/// Stop listening.
	/// </summary>
	void DgramListener::StopListening()
	{
		auto listener = _udpListener.lock();

		// Will not throw if listener has destructed.
		if (listener)
		{
			listener->StopListening();
		}
	}


	IoEndPoint DgramListener::GetPeerEndPoint() const
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Get Peer End Point Connection expired.");
		}

		stringstream addressStr;
		addressStr << listener->PeerEndPoint.address();

		IoEndPoint ep{ addressStr.str(), listener->PeerEndPoint.port() };

		return ep;
	}

	bool DgramListener::HasAsyncConnected() const
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Has Async Connected Connection expired.");
		}

		return listener->HasAsyncConnected();
	}

	/// <summary>
	/// For Unbound, unconnected listeners.
	/// </summary>
	void DgramListener::LaunchRead()
	{
		auto listener = _udpListener.lock();
		if (!listener)
		{
			throw runtime_error("DGRAM Launch Read Connection expired.");
		}

		listener->LaunchRead();
	}


	/// <summary>
	/// Checks whether the listener is valid or not.
	/// </summary>
	/// <returns>True for whether the listener is valid, false otherwise.</returns>
	bool DgramListener::IsValid() const
	{
		return !_udpListener.expired();
	}

}
