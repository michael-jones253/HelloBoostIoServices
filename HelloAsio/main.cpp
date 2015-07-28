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
#include <mutex>

using namespace HelloAsio;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;


int main(int argc, const char * argv[]) {
    // insert code here...
    
    IoServices servicesInstance{};
    
    // MJ Unless I make another wrapped instance of io_service, then the thread group
    // blocks the timers. Running timers from a second instance overcomes this.
    IoServices secondInstance{};
    mutex workMutex{};
    
    std::cout << "Hello, World!\n";
    
    auto timeout = [&]() {
        lock_guard<mutex> ioGuard(workMutex);
        cout << "Hello Timeout: " << "fix id" << endl;
    };
    
    secondInstance.SetPeriodicTimer(PeriodicTimer::General, timeout);
    
    {
        // Test scope of work outliving this block.
        auto work = [&](int id)->void {
            lock_guard<mutex> ioGuard(workMutex);
            cout << "Hello Work World: " << id << endl;
            sleep_for(seconds(2));
            cout << "Goodbye Working Life: " << id << endl;
        };
        
        for (int w = 0; w < 10; w++) {
            auto workFn = bind(work, w);
            servicesInstance.RunWork(workFn);
        }
    }
    
    for (int x = 0; x < 10; x++) {
        sleep_for(seconds(3));
        cout << "Hello" << endl;
    }
    
    return 0;
}
