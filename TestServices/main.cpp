//
//  main.cpp
//  TestServices
//
//  Created by Michael Jones on 31/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>
#include <assert.h>
#include <future>
#include <thread>
#include <chrono>

#include "IoBufferWrapper.h"
#include "IoCircularBuffer.h"

using namespace HelloAsio;
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    string msg = "Hello World!";
    IoBufferWrapper wrapper(msg);
    
    cout << "msg len: " << msg.length() << " buffer len: " << wrapper.Buffer.size() << endl;
    assert(wrapper.Buffer.size() == msg.size());
    
    const char* bufferPtr = reinterpret_cast<char*>(wrapper.Buffer.data());
    cout << "Buffer contents: " << bufferPtr << endl;
    assert(strcmp(bufferPtr, msg.data()) == 0);
    assert(msg.size() > 0);
    
    IoBufferWrapper anotherWrapper(move(wrapper.Buffer));
    auto anotherPtr = reinterpret_cast<char*>(anotherWrapper.Buffer.data());
    cout << "Another Buffer contents: " << anotherPtr << endl;
    assert(strcmp(bufferPtr, msg.data()) == 0);
    assert(wrapper.Buffer.size() == 0);
    
    IoBufferWrapper bufferFromMove(move(anotherWrapper));
    auto fromMovePtr = reinterpret_cast<char*>(bufferFromMove.Buffer.data());
    cout << "Buffer contents from move: " << fromMovePtr << endl;
    assert(strcmp(fromMovePtr, msg.data()) == 0);
    assert(anotherWrapper.Buffer.size() == 0);
    
    auto asyncReadSomeInCallback = [](uint8_t* bufPtr, ssize_t len, std::function<void(ssize_t)>&&handler) {
        memset(bufPtr, 0xFF, len);
        
        auto readSomeHandler = move(handler);
        auto deferredHandle = [&readSomeHandler, len]() {
            sleep_for(milliseconds(100));
            readSomeHandler(len);
        };
        
        auto handle = async(launch::async, move(deferredHandle));
        
    };
    
    auto notifyAvailableCallback = [](ssize_t available) {
        cout << "Available: " << available << endl;
    };

    IoCircularBuffer circularBuffer{asyncReadSomeInCallback, notifyAvailableCallback};
    circularBuffer.ReadSome();
    
    
    return 0;
}
