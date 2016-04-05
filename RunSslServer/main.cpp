//
//  main.cpp
//  RunSslServer
//
//  Created by Michael Jones on 15/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

#include "EgSslServer.hpp"
#include <openssl/err.h>
#include <boost/asio.hpp>
#include <iostream>
#include <stdlib.h>

using namespace boost::asio;
using namespace std; // For atoi.

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
        server s(io_service, atoi(argv[1]));

        io_service::work keepRunning(io_service);

        std::cout << "Hello, World!\n";
        
        s.set_context();
        s.start_accept();
        
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
