//
//  main.cpp
//  HelloAsio
//
//  Created by Michael Jones on 26/07/2015.
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
    
    IoServices servicesInstance{};
    
    std::cout << "Hello, World!\n";
    
    auto work = []()->void {
        cout << "Hello Work World" << endl;
        sleep_for(seconds(2));
        cout << "Goodbye Working Life" << endl;
    };
    
    for (int w = 0; w < 10; w++) {
        servicesInstance.RunWork(work);
    }
    
    for (int x = 0; x < 10; x++) {
        sleep_for(seconds(3));
        cout << "Hello" << endl;
    }
    
    return 0;
}
