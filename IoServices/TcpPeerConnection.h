//
//  TcpPeerConnection.h
//  AsyncIo
//
//  Created by Michael Jones on 29/07/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__TcpPeerConnection__
#define __HelloAsio__TcpPeerConnection__

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

namespace AsyncIo {
    
    class TcpPeerConnection;
    
    // using ErrorCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, boost::system::error_code)>;
	using ErrorCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, const boost::system::error_code&)>;

    using ReadSomeCallback = std::function<void(std::shared_ptr<TcpPeerConnection>, std::size_t bytesAvailable)>;

	using ConnectCallback = std::function<void(std::shared_ptr<TcpPeerConnection>)>;
    
    using BoostIoHandler = std::function<void(const boost::system::error_code& error, // Result of operation.
                                                         std::size_t bytes_transferred           // Number of bytes written from the
                                                         // buffers. If an error occurred,
                                                         // this will be less than the sum
                                                         // of the buffer sizes.
                                                         )>;

    class TcpPeerConnection : public std::enable_shared_from_this<TcpPeerConnection> {
    private:
        std::mutex _mutex;
        std::deque<std::shared_ptr<IoBufferWrapper>> mOutQueue;
        IoCircularBuffer _readBuffer;
    protected:
        ErrorCallback _errorCallback;
        ConnectCallback _connectCallback;
        
    public:
        boost::asio::ip::tcp::socket PeerSocket;
        boost::asio::ip::tcp::endpoint PeerEndPoint;

		TcpPeerConnection(boost::asio::io_service* ioService, AsyncIo::ErrorCallback&& errorCallback);
		~TcpPeerConnection() {
			std::cout << "Closing TCP peer connection: " << PeerEndPoint << std::endl;
		}
        
		// NB returns backlog.
        int AsyncWrite(std::string&& msg, bool nullTerminate);

		// NB returns backlog.
		int AsyncWrite(std::vector<uint8_t>&& msg);

		void AsyncConnect(ConnectCallback&& connectCb, std::string ipAddress, int port);
        
		void BeginChainedRead(IoNotifyAvailableCallback&& available,
			AsyncIo::ErrorCallback&& errorCallback,
			int chunkSize);
        
        boost::asio::ip::tcp::socket& GetPeerSocket() { return PeerSocket; }
        
        const uint8_t* Data() const { return _readBuffer.Get(); }
        
        size_t Size() const { return _readBuffer.Size(); }
        
        void Consume(size_t len) { _readBuffer.Consume(len); }
        
        void CopyTo(std::vector<uint8_t>& dest, int len);

		void Close();
        
    private:
        
        void LaunchWrite();
        
		void ConnectHandler(std::shared_ptr<TcpPeerConnection> conn, boost::system::error_code ec);

		void WriteHandler(
                          std::shared_ptr<TcpPeerConnection> conn,
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

#endif /* defined(__HelloAsio__TcpPeerConnection__) */
