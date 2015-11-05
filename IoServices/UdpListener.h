
#ifndef __HelloAsio__UdpListener__
#define __HelloAsio__UdpListener__

// FIX ME - probably need a later version of boost.
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "IoBufferWrapper.h"
#include "IoCircularBuffer.h"
#include <memory>
#include <mutex>
#include <functional>
#include <deque>
#include <boost/asio.hpp>
#include <vector>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo
{    
    class UdpListener;
    
	using UdpErrorCallback = std::function<void(std::shared_ptr<UdpListener>, const boost::system::error_code&)>;

    using ReceiveFromCallback = std::function<void(std::shared_ptr<UdpListener>, std::size_t bytesAvailable)>;


	/// <summary>
	/// Internal class for asynchronous receive of UDP datagrams.
	/// NB not to be instantiated by application code.
	/// </summary>
    class UdpListener final : public std::enable_shared_from_this<UdpListener>
	{
    private:
        std::mutex Mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> mOutQueue;
        UdpErrorCallback _errorCallback;
		std::function<void()> _connectCallback;
        IoCircularBuffer _readBuffer;
        
    public:
        boost::asio::ip::udp::socket PeerSocket;
        boost::asio::ip::udp::endpoint PeerEndPoint;

		UdpListener(boost::asio::io_service* ioService, const boost::asio::ip::address& address, int port);
		~UdpListener()
		{
			std::cout << "Closing UDP listener: " << PeerEndPoint << std::endl;
		}
        
		// For already bound listeners.
		void AsyncConnect(const boost::asio::ip::address& destIp, int port, std::function<void()>&& connectHandler);

        void AsyncWrite(std::string&& msg, bool nullTerminate);

		void AsyncWrite(std::vector<uint8_t>&& msg);

		void BeginChainedRead(IoNotifyAvailableCallback&& available, AsyncIo::UdpErrorCallback&& errCb, int datagramSize);
        
        boost::asio::ip::udp::socket& GetPeerSocket() { return PeerSocket; }
        
        const uint8_t* Data() const { return _readBuffer.Get(); }
        
        size_t Size() const { return _readBuffer.Size(); }
        
        void Consume(size_t len) { _readBuffer.Consume(len); }
        
        void CopyTo(std::vector<uint8_t>& dest, int len);

		void StopListening();
        
    private:
		void QueueOrWriteBuffer(std::shared_ptr<IoBufferWrapper> bufWrapper);

		void LaunchWrite();

		// For already bound listeners.
		void ConnectHandler(std::shared_ptr<UdpListener> conn, boost::system::error_code ec);

		void WriteHandler(
			std::shared_ptr<UdpListener> conn,
			std::shared_ptr<IoBufferWrapper> bufWrapper,
			boost::system::error_code ec,
			std::size_t written);
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__UdpListener__) */
