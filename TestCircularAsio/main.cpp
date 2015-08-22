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

using namespace HelloAsio;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    IoServices servicesInstance{};
    
    servicesInstance.RunTcpServer(23);
    
    

    return 0;
}
