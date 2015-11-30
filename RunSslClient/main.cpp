//
//  main.cpp
//  RunSslClient
//
//  Created by Michael Jones on 19/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

// #define BOOST_EXCEPTION_DISABLE
//#define BOOST_NO_EXCEPTIONS
// #include <boost/asio/ssl.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>

#include "TcpSslConnection.hpp"

#include <openssl/err.h>
#include <iostream>
#include <future>

using namespace AsyncIo;
using namespace boost::asio;
using namespace std;

int main(int argc, const char * argv[]) {
    try {
        // insert code here...
        // Attempt to see if loading the strings was the problem - it isn't.
        SSL_load_error_strings();
        
#if defined(BOOST_ASIO_HEADER_ONLY)
        cout << "BOOST HEADER ONLY" << endl;
#endif

        const auto& sa = boost::system::get_system_category();
        auto sn = sa.name();

        //static const boost::system::error_category& ecat = get_ssl_category();
                    
        const auto& ssl_category = boost::asio::error::get_ssl_category();
        
        // In debug mode this gives an access violation. This means that every time a boost ssl error is thrown an
        // attempt to print out message will seg fault the program. In release mode it is fine. I tried this after
        // exhaustively trying to change the order of error_category construction - they are statics with no members.
        // m_cat is a bad pointer in release.
        // Similar issue to this: http://lists.boost.org/Archives/boost/2007/10/128692.php
        auto nm = ssl_category.name();
        
        cout << "ssl_category.name: " << nm << endl;

        
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
