//
//  TcpUnixConnectionImpl.h
//  AsyncIo
//
//  Created by Michael Jones on 17/10/2016.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__TcpUnixConnectionImpl__
#define __HelloAsio__TcpUnixConnectionImpl__

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
    
    class TcpUnixConnectionImpl;
    
    // using ErrorCallback = std::function<void(std::shared_ptr<TcpUnixConnectionImpl>, boost::system::error_code)>;
	using ErrorCallback = std::function<void(std::shared_ptr<TcpUnixConnectionImpl>, const boost::system::error_code&)>;

    using ReadSomeCallback = std::function<void(std::shared_ptr<TcpUnixConnectionImpl>, std::size_t bytesAvailable)>;

	using ConnectCallback = std::function<void(std::shared_ptr<TcpUnixConnectionImpl>)>;
    
    using BoostIoHandler = std::function<void(const boost::system::error_code& error, // Result of operation.
                                                         std::size_t bytes_transferred           // Number of bytes written from the
                                                         // buffers. If an error occurred,
                                                         // this will be less than the sum
                                                         // of the buffer sizes.
                                                         )>;

    class TcpUnixConnectionImpl : public std::enable_shared_from_this<TcpUnixConnectionImpl> {
    private:
        std::mutex _mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> mOutQueue;
        IoCircularBuffer _readBuffer;
    protected:
        ErrorCallback _errorCallback;
        ConnectCallback _connectCallback;
        
    public:
        boost::asio::local::stream_protocol::socket UnixSocket;
        boost::asio::local::stream_protocol::endpoint UnixEndPoint;

		TcpUnixConnectionImpl(boost::asio::io_service* ioService, AsyncIo::ErrorCallback&& errorCallback);
		~TcpUnixConnectionImpl() {
			std::cout << "Closing Unix domain connection: " << UnixEndPoint << std::endl;
		}
        
		// NB returns backlog.
        int AsyncWrite(std::string&& msg, bool nullTerminate);

		// NB returns backlog.
		int AsyncWrite(std::vector<uint8_t>&& msg);

		void AsyncConnect(ConnectCallback&& connectCb, const std::string& name);
        
		void BeginChainedRead(IoNotifyAvailableCallback&& available,
			AsyncIo::ErrorCallback&& errorCallback,
			int chunkSize);
        
        boost::asio::local::stream_protocol::socket& GetUnixSocket() { return UnixSocket; }
        
        const uint8_t* Data() const { return _readBuffer.Get(); }
        
        size_t Size() const { return _readBuffer.Size(); }
        
        void Consume(size_t len) { _readBuffer.Consume(len); }
        
        void CopyTo(std::vector<uint8_t>& dest, int len);

		void Close();
        
    private:
        
        void LaunchWrite();
        
		void ConnectHandler(std::shared_ptr<TcpUnixConnectionImpl> conn, boost::system::error_code ec);

		void WriteHandler(
                          std::shared_ptr<TcpUnixConnectionImpl> conn,
                          std::shared_ptr<IoBufferWrapper> bufWrapper,
                          boost::system::error_code ec,
                          std::size_t written);
        
        virtual void AsyncWriteToSocket(std::shared_ptr<IoBufferWrapper>& bufferWrapper, BoostIoHandler&& handler) = 0;

        virtual void AsyncReadSome(uint8_t* bufPtr, size_t len, BoostIoHandler&& handler) = 0;
        
        virtual void UpperLayerHandleConnect() = 0;
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__TcpUnixConnectionImpl__) */
