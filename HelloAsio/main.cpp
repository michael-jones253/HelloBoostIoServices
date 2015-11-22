//
//  main.cpp
//  HelloAsio
//
//  Created by Michael Jones on 26/07/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "IoServices.h"
#include "TestPeriodicTimer.h"
#include "TestTimer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <array>
#include <assert.h>

using namespace AsyncIo;
using namespace TestAsyncIo;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;


int main(int argc, const char * argv[]) {
    // insert code here...
    
    auto errLogger = [](const std::string& msg, const std::exception& ex) {
        cout << "IO services exception: " << ex.what() << " " << msg << endl;
    };

    IoServices servicesInstance{ move(errLogger) };
    
    servicesInstance.Start();
    
    // servicesInstance.RunTcpServer(23);
    
    auto errLogger2 = [](const std::string& msg, const std::exception& ex) {
        cout << "IO services exception: " << ex.what() << " " << msg << endl;
    };

    // MJ Unless I make another wrapped instance of io_service, then the thread group
    // blocks the timers. Running timers from a second instance overcomes this.
    IoServices secondInstance{ move(errLogger2) };
    
    secondInstance.Start();
    
    auto readStream = [](shared_ptr<StreamConnection> conn, int bytesAvailable) {
        const int amountForConsume = 25;
        array<uint8_t, amountForConsume + 1> buf{};
        cout << "AVAILABLE IN SERVER: " << bytesAvailable << endl;
        if (bytesAvailable >= amountForConsume) {
            assert(static_cast<int>(conn->Size()) == bytesAvailable);
            
            // Does this put a null on the end?
            string msgFromData(reinterpret_cast<const char*>(conn->Data()), amountForConsume);
            
            cout << "MSG FROMDATA: " << msgFromData << endl;
            
            conn->ConsumeInto(buf.data(), amountForConsume);
            // Probably not needed.
            buf[amountForConsume] = '\0';
            string msgFromBuf(reinterpret_cast<char*>(buf.data()), amountForConsume);
            
            assert(msgFromData.compare(msgFromBuf) == 0);
            
            cout << "MSG: " << msgFromBuf << endl;
        }
    };
    
    auto clientConnected = [](shared_ptr<StreamConnection> conn) {
        
    };

    secondInstance.AddTcpServer(23, move(clientConnected), move(readStream));
    secondInstance.StartTcpServer(23);
    mutex workMutex{};
    
    std::cout << "Hello, World!\n";
    
    int generalCount{};
    int secondaryCount{};
    
    auto timeout = [&](PeriodicTimer id) {
        switch (id.Id) {
            case static_cast<int>(TestPeriodicTimer::General):
                generalCount++;
                break;
                
            case static_cast<int>(TestPeriodicTimer::Secondary):
                secondaryCount++;
                break;
                
            default:
                break;
        }
        lock_guard<mutex> ioGuard(workMutex);
        cout << "Hello Timeout id: " << id << endl;
    };
    
    secondInstance.SetPeriodicTimer(ToId(TestPeriodicTimer::General), seconds(1), timeout);
    secondInstance.SetPeriodicTimer(ToId(TestPeriodicTimer::Secondary), seconds(2), timeout);
    
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
        secondInstance.SendToAllServerConnections("Hello World!", true);
        secondInstance.SendToAllServerConnections("Hello World!", true);
        secondInstance.SendToAllServerConnections("Hello World!", true);
    }
    
    cout << "General count: " << generalCount << endl;
    cout << "Secondary count " << secondaryCount << endl;
    
    return 0;
}
