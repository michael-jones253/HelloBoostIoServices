//
//  main.cpp
//  TestServices
//
//  Created by Michael Jones on 31/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>
#include <assert.h>

#include "IoBufferWrapper.h"

using namespace HelloAsio;
using namespace std;

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
    
    return 0;
}
