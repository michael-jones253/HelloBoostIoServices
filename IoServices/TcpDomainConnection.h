//
//  TcpDomainConnection.h
//  AsyncIo
//
//  Created by Michael Jones on 17/10/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__TcpDomainConnection__
#define __HelloAsio__TcpDomainConnection__

//#define _WINSOCK_DEPRECATED_NO_WARNINGS

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

namespace AsyncIo {
    
    class TcpDomainConnection;
    
	using DomainErrorCallback = std::function<void(std::shared_ptr<TcpDomainConnection>, const boost::system::error_code&)>;

    using DomainReadSomeCallback = std::function<void(std::shared_ptr<TcpDomainConnection>, std::size_t bytesAvailable)>;

	using DomainConnectCallback = std::function<void(std::shared_ptr<TcpDomainConnection>)>;
    
    using BoostIoHandler = std::function<void(const boost::system::error_code& error, // Result of operation.
                                                         std::size_t bytes_transferred           // Number of bytes written from the
                                                         // buffers. If an error occurred,
                                                         // this will be less than the sum
                                                         // of the buffer sizes.
                                                         )>;

    class TcpDomainConnection : public std::enable_shared_from_this<TcpDomainConnection> {
    private:
        std::mutex _mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> mOutQueue;
        IoCircularBuffer _readBuffer;
    protected:
        DomainErrorCallback _errorCallback;
        DomainConnectCallback _connectCallback;
        
    public:
        boost::asio::local::stream_protocol::socket DomainSocket;
        boost::asio::local::stream_protocol::endpoint DomainEndPoint;

		TcpDomainConnection(boost::asio::io_service* ioService, AsyncIo::DomainErrorCallback&& errorCallback);
		~TcpDomainConnection() {
			std::cout << "Closing Unix domain connection: " << DomainEndPoint << std::endl;
		}
        
		// NB returns backlog.
        int AsyncWrite(std::string&& msg, bool nullTerminate);

		// NB returns backlog.
		int AsyncWrite(std::vector<uint8_t>&& msg);

		void AsyncConnect(DomainConnectCallback&& connectCb, const std::string& name);
        
		void BeginChainedRead(IoNotifyAvailableCallback&& available,
			AsyncIo::DomainErrorCallback&& errorCallback,
			int chunkSize);
        
        boost::asio::local::stream_protocol::socket& GetDomainSocket() { return DomainSocket; }
        
        const uint8_t* Data() const { return _readBuffer.Get(); }
        
        size_t Size() const { return _readBuffer.Size(); }
        
        void Consume(size_t len) { _readBuffer.Consume(len); }
        
        void CopyTo(std::vector<uint8_t>& dest, int len);

		void Close();
        
    private:
        
        void LaunchWrite();
        
		void ConnectHandler(std::shared_ptr<TcpDomainConnection> conn, boost::system::error_code ec);

		void WriteHandler(
                          std::shared_ptr<TcpDomainConnection> conn,
                          std::shared_ptr<IoBufferWrapper> bufWrapper,
                          boost::system::error_code ec,
                          std::size_t written);
        
        void AsyncReadSome(uint8_t* bufPtr, size_t len, BoostIoHandler&& handler);
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__TcpDomainConnection__) */
