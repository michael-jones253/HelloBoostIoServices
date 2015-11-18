//
//  EgSslServer.hpp
//  HelloAsio
//
//  Created by Michael Jones on 15/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

#ifndef EgSslServer_hpp
#define EgSslServer_hpp

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

class session
{
private:
    ssl_socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];

public:
    session(boost::asio::io_service& io_service,
            boost::asio::ssl::context& context);
    ssl_socket::lowest_layer_type& socket();
    void start();
    void handle_handshake(const boost::system::error_code& error);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
};

class server
{
private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ssl::context context_;
public:
    server(boost::asio::io_service& io_service, unsigned short port);
    
    void set_context();
    
    std::string get_password() const;
    void start_accept();
    void handle_accept(session* new_session, const boost::system::error_code& error);
    
};
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif
#endif /* EgSslServer_hpp */
