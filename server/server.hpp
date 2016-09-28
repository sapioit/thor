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
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace http {
namespace server {

/// The top-level class of the HTTP server.
class server : private boost::noncopyable {
    public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit server(const std::string &address, const std::string &http_port, const std::string &https_port,
                    const std::string &doc_root, const std::string &cert_root, const std::string &compression_folder,
                    std::size_t thread_pool_size, const std::vector<user_handler> &user_handlers);

    /// Run the server's io_service loop.
    void run();

    private:
    /// Initiate an asynchronous accept operation.
    void start_accept();

    void start_ssl_accept();

    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code &e);

    void handle_ssl_accept(const boost::system::error_code &e);

    std::string get_cert_folder() const;

    std::string get_compression_folder() const;

    std::string get_password() const;

    /// Handle a request to stop the server.
    void handle_stop();

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
