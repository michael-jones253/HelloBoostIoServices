//
//  TcpConnection.hpp
//  HelloAsio
//
//  Created by Michael Jones on 5/04/2016.
//  Copyright © 2016 Michael Jones. All rights reserved.
//

#ifndef TcpConnection_hpp
#define TcpConnection_hpp

#include "TcpPeerConnection.h"
#include <stdio.h>

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo {

class TcpConnection final : public TcpPeerConnection {
public:
    TcpConnection(boost::asio::io_service* ioService, AsyncIo::ErrorCallback&& errorCallback);

private:
    void AsyncWriteToSocket(std::shared_ptr<IoBufferWrapper>& bufferWrapper, BoostIoHandler&& handler) override;
    void AsyncReadSome(uint8_t* bufPtr, size_t len, BoostIoHandler&& handler) override;
    void UpperLayerHandleConnect() override;
};
}

#if defined(__GNUC__)
#pragma GCC visibility pop
#endif
#endif /* TcpConnection_hpp */
