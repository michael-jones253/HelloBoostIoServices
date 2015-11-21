//
//  EgSslServer.cpp
//  HelloAsio
//
//  Created by Michael Jones on 15/11/2015.
//  Copyright © 2015 Michael Jones. All rights reserved.
//

#include "EgSslServer.hpp"
#include <string>

server::server(boost::asio::io_service& io_service, unsigned short port)
: io_service_(io_service),
acceptor_(io_service,
          boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          context_(boost::asio::ssl::context::sslv23)
{
    /*
     */
    
    // start_accept();
}

void server::set_context() {
    context_.set_options(
                         boost::asio::ssl::context::default_workarounds
                         | boost::asio::ssl::context::no_sslv2
                         | boost::asio::ssl::context::single_dh_use);
    context_.set_password_callback(boost::bind(&server::get_password, this));
    context_.use_certificate_chain_file("fd-serv.crt");
    context_.use_rsa_private_key_file("fd-rsa-priv.key", boost::asio::ssl::context::pem);
    context_.use_tmp_dh_file("dh1024.pem");
}

std::string server::get_password() const
{
    std::cout << "Getting password" << std::endl;
    return "hello";
}

void server::start_accept()
{
    session* new_session = new session(io_service_, context_);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session,
                           const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "Accepting client!!!!" << std::endl;
        new_session->start();
    }
    else
    {
        delete new_session;
    }
    
    start_accept();
}

session::session(boost::asio::io_service& io_service,
                 boost::asio::ssl::context& context)
: socket_(io_service, context)
{
}

ssl_socket::lowest_layer_type& session::socket()
{
    return socket_.lowest_layer();
}

void session::start()
{
    socket_.async_handshake(boost::asio::ssl::stream_base::server,
                            boost::bind(&session::handle_handshake, this,
                                        boost::asio::placeholders::error));
}

void session::handle_handshake(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "handle handshake" << std::endl;
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        SSL_load_error_strings();
        unsigned long n = ERR_get_error();
        char buf[1024];
        printf("%s\n", ERR_error_string(n, buf));

        std::cout << "deleting client" /* << error.message() */ << std::endl;
        delete this;
    }
}

void session::handle_read(const boost::system::error_code& error,
                          size_t bytes_transferred)
{
    if (!error)
    {
        std::cout << std::string(data_, bytes_transferred) << std::endl;
        
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(data_, bytes_transferred),
                                 boost::bind(&session::handle_write, this,
                                             boost::asio::placeholders::error));
    }
    else
    {
        std::cout << "read error" << std::endl;
        delete this;
    }
}

void session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}
