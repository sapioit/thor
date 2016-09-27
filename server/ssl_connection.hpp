//
// ssl_connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SSL_CONNECTION
#define SSL_CONNECTION
#include "connection.hpp"
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace http {
namespace server {

/// Represents a single connection from a client.
class ssl_connection : public connection {
    public:
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;
    /// Construct a connection with the given io_service.
    explicit ssl_connection(boost::asio::io_service &io_service, boost::asio::ssl::context &context,
                            request_handler &handler);

    virtual ~ssl_connection();

    ssl_socket::lowest_layer_type &lowest_layer__socket() { return socket_.lowest_layer(); }

    /// Start the first asynchronous operation for the connection.
    void start() override;

    private:
    void sync_read(char *where, std::size_t bytes, boost::system::error_code &ec) override;

    protected:
    virtual void start_reading(const boost::system::error_code &error = {}) override;

    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) override;

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code &e) override;

    void handle_shutdown(const boost::system::error_code &);

    void print_err(boost::system::error_code error);

    boost::shared_ptr<ssl_connection> shared_from_this();

    private:
    /// Socket for the connection.
    ssl_socket socket_;
};

typedef boost::shared_ptr<ssl_connection> ssl_connection_ptr;

} // namespace server3
} // namespace http

#endif // SSL_CONNECTION
