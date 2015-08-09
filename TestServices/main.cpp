//
//  main.cpp
//  TestServices
//
//  Created by Michael Jones on 31/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoBufferWrapper.h"
#include "IoCircularBuffer.h"

#include <iostream>
#include <assert.h>
#include <future>
#include <thread>
#include <chrono>
#include <list>
#include <array>
#include <string.h>


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
    
    list<future<bool>> handles{};
    
    auto asyncReadSomeInCallback = [&handles](uint8_t* bufPtr, ssize_t len, std::function<void(ssize_t)>&&handler) {
        
        auto readSomeHanderl = move(handler);
        auto deferredHandle = [=]() -> bool {
            sleep_for(milliseconds(100));
            memset(bufPtr, 0xFF, len);
            cout << "Buf Ptr: " << reinterpret_cast<long long>(bufPtr) << endl;

            readSomeHanderl(len);
            return true;
        };
        
        auto handle = async(launch::async, move(deferredHandle));
        handles.push_back(move(handle));
    };
    
    int amountForConsume{24};
    const int AmountForCompare{1024};
    array<uint8_t, AmountForCompare> patternForCompare{};
    memset(patternForCompare.data(), 0xFF, AmountForCompare);
    
    IoCircularBuffer circularBuffer{asyncReadSomeInCallback};
    
    auto notifyAvailableCallback = [&](ssize_t available) {
        cout << "Available: " << available << endl;
        if (available >= amountForConsume) {
            assert(memcmp(circularBuffer.Get(), patternForCompare.data(), available) == 0);
            
            // We can do what we want with the memory to consume - test replenish of pattern from read some.
            memset(const_cast<uint8_t*>(circularBuffer.Get()), 0x00, amountForConsume);
            circularBuffer.Consume(amountForConsume);
        }
    };
    
    const int chunkSize = 12;
    circularBuffer.BeginReadSome(move(notifyAvailableCallback), chunkSize);
    
    int count{};
    while (true) {
        sleep_for(seconds(3));
        cout << "Main looping" << endl;
        if (++count == 10) {
            // Test partial consume.
            amountForConsume -= 2;
        }
    }
    
    return 0;
}
