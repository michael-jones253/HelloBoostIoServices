
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

namespace HelloAsio {
    
    class UdpListener;
    
	using UdpErrorCallback = std::function<void(std::shared_ptr<UdpListener>, const boost::system::error_code&)>;

    using ReceiveFromCallback = std::function<void(std::shared_ptr<UdpListener>, std::size_t bytesAvailable)>;

    class UdpListener final : public std::enable_shared_from_this<UdpListener> {
    private:
        std::mutex Mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> OutQueue;
        const UdpErrorCallback _errorCallback;
        IoCircularBuffer _readBuffer;
        
    public:
        boost::asio::ip::udp::socket PeerSocket;
        boost::asio::ip::udp::endpoint PeerEndPoint;

		UdpListener(boost::asio::io_service* ioService, HelloAsio::UdpErrorCallback&& errorCallback, int port);
		~UdpListener() {
			std::cout << "Closing UDP peer connection: " << PeerEndPoint << std::endl;
		}
        
        void AsyncWrite(std::string&& msg, bool nullTerminate);

		void BeginChainedRead(IoNotifyAvailableCallback&& available, int datagramSize);
        
        boost::asio::ip::udp::socket& GetPeerSocket() { return PeerSocket; }
        
        const uint8_t* Data() const { return _readBuffer.Get(); }
        
        size_t Size() const { return _readBuffer.Size(); }
        
        void Consume(size_t len) { _readBuffer.Consume(len); }
        
        void CopyTo(std::vector<uint8_t>& dest, int len);
        
    private:
        
        void LaunchWrite();
        
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
