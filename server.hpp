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

#include "connection.hpp"
#include "request_handler.hpp"
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <future>
#include <string>
#include <vector>

namespace http {
namespace server {

/// The top-level class of the HTTP server.
class server : private boost::noncopyable {
    public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit server(const std::string &address, const std::string &port, const std::string &doc_root,
                    std::size_t thread_pool_size, const std::vector<user_handler> &user_handlers)
        : thread_pool_size_(thread_pool_size), signals_(io_service_), acceptor_(io_service_), new_connection_(),
          request_handler_(doc_root, user_handlers) {
        // Register to handle the signals that indicate when the server should exit.
        // It is safe to register for the same signal multiple times in a program,
        // provided all registration for the specified signal is made through Asio.
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
        signal(SIGPIPE, SIG_IGN);
#if defined(SIGQUIT)
        signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
        signals_.async_wait(std::bind(&server::handle_stop, this));

        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(address, port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        start_accept();
    }

    /// Run the server's io_service loop.
    void run() {
        // Create a pool of threads to run all of the io_services.
        std::vector<std::future<void>> tasks(thread_pool_size_);
        for (auto &task : tasks) {
            task = std::async(std::launch::async, [this]() { io_service_.run(); });
        }

        // std::future destructor waits for all threads in the pool to exit.
    }

    private:
    /// Initiate an asynchronous accept operation.
    void start_accept() {
        new_connection_.reset(new connection(io_service_, request_handler_));
        acceptor_.async_accept(new_connection_->socket(),
                               std::bind(&server::handle_accept, this, std::placeholders::_1));
    }

    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code &e) {
        if (!e) {
            new_connection_->start();
        }

        start_accept();
    }

    /// Handle a request to stop the server.
    void handle_stop() { io_service_.stop(); }

    /// The number of threads that will call io_service::run().
    std::size_t thread_pool_size_;

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service io_service_;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// The next connection to be accepted.
    connection_ptr new_connection_;

    /// The handler for all incoming requests.
    request_handler request_handler_;
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_SERVER_HPP
