//
//  main.cpp
//  TestCircularAsio
//
//  Created by Michael Jones on 22/08/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoServices.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace AsyncIo;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    auto errLogger = [](const std::string& msg, const std::exception&) {
        
    };
    
    IoServices servicesInstance{ move(errLogger) };
    
    auto readCb = [](std::shared_ptr<StreamConnection> conn, int bytesAvailable) {
        
    };
    
    auto acceptStream = [](std::shared_ptr<StreamConnection> conn) {
        
    };
    
    servicesInstance.AddTcpServer(23, move(acceptStream), move(readCb));
    servicesInstance.StartTcpServer(23);
    
    while (true) {
        sleep_for(seconds(3));
    }
    

    return 0;
}
