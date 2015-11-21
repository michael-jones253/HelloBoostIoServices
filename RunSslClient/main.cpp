//
//  main.cpp
//  RunSslClient
//
//  Created by Michael Jones on 19/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

#include "TcpSslConnection.hpp"

#include <openssl/err.h>
#include <boost/asio.hpp>
#include <iostream>
#include <future>

using namespace AsyncIo;
using namespace boost::asio;
using namespace std;

int main(int argc, const char * argv[]) {
    try {
        // insert code here...
        

        boost::asio::io_service io_service;
        io_service::work keepRunning(io_service);
        
        // boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv3);
        
        // this should be a list of certificates that are trusted.
        ctx.load_verify_file("ca.pem"); // calls SSL_CTX_load_verify_locations
        
        auto errCb = [](std::shared_ptr<TcpSslConnection>, const boost::system::error_code& ec) {
 
            SSL_load_error_strings();
            unsigned long n = ERR_get_error();
            char buf[1024];
            printf("%s\n", ERR_error_string(n, buf));
                       cout << "CLIENT ERROR" << ec.message() << endl;
        };
        
        shared_ptr<TcpSslConnection> client = make_shared<TcpSslConnection>(&io_service, move(ctx), move(errCb));
        future<bool> inputHandle{};
        
        auto notifyData = [](std::shared_ptr<TcpSslConnection> clientConn, size_t available) {
            cout << "Available: " << available << endl;
        };
        
        auto connectCb = [&inputHandle, &notifyData, &errCb](std::shared_ptr<TcpSslConnection> clientConn) {
            cout << "CLIENT CONN!!!" << endl;
            
            auto availableCb = [clientConn, &notifyData](size_t available) {
                notifyData(clientConn, available);
            };
            
            clientConn->BeginChainedRead(move(availableCb), move(errCb), 12);
            
            auto inputLoop = [clientConn]() {
                while (true) {
                    cout << "enter text: ";
                    string msg{};
                    cin >> msg;
                    clientConn->AsyncWrite(move(msg), true);
                }
                
                
                return true;
            };
            
            inputHandle = async(launch::async, move(inputLoop));
            
        };
        
        client->AsyncConnect(move(connectCb), "127.0.0.1", atoi(argv[1]));
        
        std::cout << "Hello, World!\n";
        
        
        io_service.run();
    }
    catch (std::exception& e)
    {
        SSL_load_error_strings();
        unsigned long n = ERR_get_error();
        char buf[1024];
        printf("%s\n", ERR_error_string(n, buf));
        std::cerr << "Exception: " << e.what() << "\n";
        
    }
    
    return 0;
}
