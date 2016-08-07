//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_SERVER_HPP
#define HTTP_SERVER3_SERVER_HPP

#include "io_service_pool.hpp"
#include "request_handler.hpp"
#include "ssl_connection.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <future>
#include <stdlib.h>
#include <string>
#include <vector>

namespace http {
namespace server {

/// The top-level class of the HTTP server.
class server : private boost::noncopyable {
    public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit server(const std::string &address, const std::string &http_port, const std::string &https_port,
                    const std::string &doc_root, const std::string &cert_root, std::size_t thread_pool_size,
                    const std::vector<user_handler> &user_handlers)
        : io_service_pool_(thread_pool_size), signals_(io_service_pool_.get_io_service()),
          acceptor_(io_service_pool_.get_io_service()), ssl_acceptor_(io_service_pool_.get_io_service()),
          new_connection_(), request_handler_(doc_root, user_handlers), cert_root_(cert_root),
          ssl_context_(io_service_pool_.get_io_service(), boost::asio::ssl::context::tlsv12) {
        // Register to handle the signals that indicate when the server should exit.
        // It is safe to register for the same signal multiple times in a program,
        // provided all registration for the specified signal is made through Asio.
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
        signal(SIGPIPE, SIG_IGN);
#if defined(SIGQUIT)
        signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
        signals_.async_wait([this](const auto &, const auto &) { this->handle_stop(); });

        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::resolver resolver(io_service_pool_.get_io_service());
        boost::asio::ip::tcp::resolver::query query(address, http_port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

        boost::asio::ip::tcp::resolver ssl_resolver(io_service_pool_.get_io_service());
        boost::asio::ip::tcp::resolver::query ssl_query(address, https_port);
        boost::asio::ip::tcp::endpoint ssl_endpoint = *ssl_resolver.resolve(ssl_query);

        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();
        ssl_acceptor_.open(ssl_endpoint.protocol());
        ssl_acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        ssl_acceptor_.bind(ssl_endpoint);
        ssl_acceptor_.listen();

        ssl_context_.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 |
                                 boost::asio::ssl::context::single_dh_use);
        ssl_context_.set_password_callback([this](std::size_t, const auto &) { return this->get_password(); });
        auto cert_folder = get_cert_folder();
        ssl_context_.use_certificate_chain_file(cert_folder + "/server.crt");
        ssl_context_.use_private_key_file(cert_folder + "/server.key", boost::asio::ssl::context::pem);
        ssl_context_.use_tmp_dh_file(cert_folder + "/dh2048.pem");

        start_accept();
        start_ssl_accept();
    }

    /// Run the server's io_service loop.
    void run() { io_service_pool_.run(); }

    private:
    /// Initiate an asynchronous accept operation.
    void start_accept() {
        new_connection_.reset(new connection(io_service_pool_.get_io_service(), request_handler_));
        acceptor_.async_accept(new_connection_->socket(), [this](const auto &e) { this->handle_accept(e); });
    }

    void start_ssl_accept() {
        new_ssl_connection_.reset(
            new ssl_connection(io_service_pool_.get_io_service(), ssl_context_, request_handler_));
        ssl_acceptor_.async_accept(new_ssl_connection_->lowest_layer__socket(),
                                   [this](const auto &e) { this->handle_ssl_accept(e); });
    }

    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code &e) {
        if (!e) {
            new_connection_->start();
        }

        start_accept();
    }

    void handle_ssl_accept(const boost::system::error_code &e) {
        if (!e) {
            new_ssl_connection_->start();
        }

        start_ssl_accept();
    }

    std::string get_cert_folder() const { return cert_root_; }

    std::string get_password() const { return "test"; }

    /// Handle a request to stop the server.
    void handle_stop() { io_service_pool_.stop(); }

    /// The pool of io_service objects used to perform asynchronous operations.
    io_service_pool io_service_pool_;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    /// Acceptors used to listen for incoming connectionss.
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::acceptor ssl_acceptor_;

    /// The next connection to be accepted.
    connection_ptr new_connection_;
    ssl_connection_ptr new_ssl_connection_;

    /// The handler for all incoming requests.
    request_handler request_handler_;

    /// The folder containing the certificate files. Must have the following files:
    /// server.crt
    /// server.key
    /// dh2048.pem

    std::string cert_root_;

    /// The SSL context
    boost::asio::ssl::context ssl_context_;
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_SERVER_HPP
