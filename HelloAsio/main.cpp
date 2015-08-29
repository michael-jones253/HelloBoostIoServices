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
#include <array>
#include <assert.h>

using namespace HelloAsio;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;


int main(int argc, const char * argv[]) {
    // insert code here...
    
    IoServices servicesInstance{};
    
    // servicesInstance.RunTcpServer(23);
    
    // MJ Unless I make another wrapped instance of io_service, then the thread group
    // blocks the timers. Running timers from a second instance overcomes this.
    IoServices secondInstance{};
    
    auto readStream = [](shared_ptr<StreamConnection> conn, int bytesAvailable) {
        const int amountForConsume = 25;
        array<uint8_t, amountForConsume> buf{};
        cout << "AVAILABLE IN SERVER: " << bytesAvailable << endl;
        if (bytesAvailable >= amountForConsume) {
            assert(static_cast<int>(conn->Size()) == bytesAvailable);
            string msgFromData(reinterpret_cast<const char*>(conn->Data()), amountForConsume);
            
            cout << "MSG FROMDATA: " << msgFromData << endl;
            
            conn->ConsumeInto(buf.data(), amountForConsume);
            string msgFromBuf(reinterpret_cast<char*>(buf.data()), amountForConsume);
            
            assert(msgFromData.compare(msgFromBuf) == 0);
            
            cout << "MSG: " << msgFromBuf << endl;
        }
    };

    secondInstance.RunTcpServer(23, move(readStream));
    mutex workMutex{};
    
    std::cout << "Hello, World!\n";
    
    int generalCount{};
    int secondaryCount{};
    
    auto timeout = [&](PeriodicTimer id) {
        switch (id) {
            case PeriodicTimer::General:
                generalCount++;
                break;
                
            case PeriodicTimer::Secondary:
                secondaryCount++;
                break;
                
            default:
                break;
        }
        lock_guard<mutex> ioGuard(workMutex);
        cout << "Hello Timeout id: " << id << endl;
    };
    
    secondInstance.SetPeriodicTimer(PeriodicTimer::General, seconds(1), timeout);
    secondInstance.SetPeriodicTimer(PeriodicTimer::Secondary, seconds(2), timeout);
    
    {
        // Test scope of work outliving this block.
        auto work = [&](int id)->void {
            {
                lock_guard<mutex> ioGuard(workMutex);
                cout << "Hello Work World: " << id << endl;
            }
            
            sleep_for(seconds(2));

            lock_guard<mutex> ioGuard(workMutex);
            cout << "Goodbye Working Life: " << id << endl;
        };
        
        for (int w = 0; w < 10; w++) {
            auto workFn = bind(work, w);
            servicesInstance.RunWork(workFn);
        }
    }
    
    for (int x = 0; x < 100; x++) {
        sleep_for(seconds(3));
        cout << "Hello" << endl;
        secondInstance.HelloAllPeers();
        secondInstance.HelloAllPeers();
        secondInstance.HelloAllPeers();
    }
    
    cout << "General count: " << generalCount << endl;
    cout << "Secondary count " << secondaryCount << endl;
    
    return 0;
}
