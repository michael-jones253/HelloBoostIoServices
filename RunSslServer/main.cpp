//
//  main.cpp
//  RunSslServer
//
//  Created by Michael Jones on 15/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

#include "StreamConnection.h"
#include "TcpServer.h"
#include <openssl/err.h>
#include <boost/asio.hpp>
#include <iostream>
#include <stdlib.h>

using namespace boost::asio;
using namespace std; // For atoi.
using namespace AsyncIo;

int main(int argc, const char * argv[]) {
    // insert code here...
    try {
        if (argc != 2)
        {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        
        cout << "Port: " << argv[1] << endl;
        boost::asio::io_service io_service;
        //server s(io_service, atoi(argv[1]));
        
        auto ReadStreamCallback = [](std::shared_ptr<StreamConnection>, int bytesAvailable) {
            cout << "got bytes: " << bytesAvailable << endl;
        };
        
        auto AcceptStreamCallback = [](std::shared_ptr<StreamConnection> acceptedConn) {
            
        };
        
        auto port = atoi(argv[1]);
        
        TcpServer server(&io_service, port, move(AcceptStreamCallback), move(ReadStreamCallback));
        io_service::work keepRunning(io_service);
        
        std::cout << "Hello, World!\n";
        
        auto getPwd = []() -> std::string {
            return "hello";
        };
        
        SecurityOptions options{};
        
        options.CertificateFilename = std::string("fd-serv.crt"),
        options.PrivKeyFilename =   std::string("fd-rsa-priv.key"),
        options.DHExchangeFilename =   std::string("dh1024.pem"),
        options.GetPasswordCallback =  getPwd;
        
        
        server.Start(move(options));
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
