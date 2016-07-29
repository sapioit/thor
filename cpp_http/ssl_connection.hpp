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
                            request_handler &handler)
        : connection(io_service, handler), socket_(io_service, context) {}

    virtual ~ssl_connection() {}

    ssl_socket::lowest_layer_type &lowest_layer__socket() { return socket_.lowest_layer(); }

    /// Start the first asynchronous operation for the connection.
    void start() override {
        socket_.async_handshake(
            boost::asio::ssl::stream_base::server,
            strand_.wrap(std::bind(&ssl_connection::handle_handshake, shared_from_this(), std::placeholders::_1)));
    }

    private:
    void handle_handshake(const boost::system::error_code &error) {
        if (!error) {
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    strand_.wrap(std::bind(&ssl_connection::handle_read, shared_from_this(),
                                                           std::placeholders::_1, std::placeholders::_2)));
        }
    }

    void sync_read(char *where, std::size_t bytes, boost::system::error_code &ec) override {
        boost::asio::read(socket_, boost::asio::buffer(where, bytes),
                          [bytes](const boost::system::error_code &, std::size_t bytes_read) -> std::size_t {
                              return bytes - bytes_read;
                          },
                          ec);
    }

    protected:
    void keep_alive() override {
        socket_.async_read_some(boost::asio::buffer(buffer_),
                                strand_.wrap(std::bind(&ssl_connection::handle_read, shared_from_this(),
                                                       std::placeholders::_1, std::placeholders::_2)));
    }

    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) override {
        if (!e) {
            boost::tribool result;
            boost::tie(result, boost::tuples::ignore) =
                request_parser_.parse(request_, buffer_.data(), buffer_.data() + bytes_transferred);

            request_.read_body_func = [this]() {
                try {
                    boost::system::error_code ec;
                    drain_body(ec);
                    if (ec)
                        throw std::system_error{errno, std::system_category()};
                } catch (...) {
                    throw;
                }
            };

            if (result) {
                // The request is complete.
                request_handler_.handle_request<request_handler::protocol_type::https>(request_, reply_);
                drain_body_if_needed();
                boost::asio::async_write(
                    socket_, reply_.to_buffers(),
                    strand_.wrap(std::bind(&ssl_connection::handle_write, shared_from_this(), std::placeholders::_1)));
            } else if (!result) {
                // The request is malformed.
                reply_ = reply::stock_reply(reply::status_type::bad_request);
                drain_body_if_needed();
                boost::asio::async_write(
                    socket_, reply_.to_buffers(),
                    strand_.wrap(std::bind(&ssl_connection::handle_write, shared_from_this(), std::placeholders::_1)));
            } else {
                // Need more data.
                socket_.async_read_some(boost::asio::buffer(buffer_),
                                        strand_.wrap(std::bind(&ssl_connection::handle_read, shared_from_this(),
                                                               std::placeholders::_1, std::placeholders::_2)));
            }
        }

        // If an error occurs then no new asynchronous operations are started. This
        // means that all shared_ptr references to the connection object will
        // disappear and the object will be destroyed automatically after this
        // handler returns. The connection class's destructor closes the socket.
    }

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code &e) override {
        if (!e) {
            keep_alive_if_needed();
        } else {
            print_err(e);
        }
        // No new asynchronous operations are started. This means that all shared_ptr
        // references to the connection object will disappear and the object will be
        // destroyed automatically after this handler returns. The connection class's
        // destructor closes the socket.
    }

    void handle_shutdown(const boost::system::error_code &) {}

    void print_err(boost::system::error_code error) {
        std::string err = error.message();
        if (error.category() == boost::asio::error::get_ssl_category()) {
            err = std::string(" (") + std::to_string(ERR_GET_LIB(error.value())) + "," +
                  std::to_string(ERR_GET_FUNC(error.value())) + "," + std::to_string(ERR_GET_REASON(error.value())) +
                  ") ";
            // ERR_PACK /* crypto/err/err.h */
            char buf[128];
            ::ERR_error_string_n(error.value(), buf, sizeof(buf));
            err += buf;
        }
        std::cout << err << std::endl;
    }

    boost::shared_ptr<ssl_connection> shared_from_this() {
        return boost::dynamic_pointer_cast<ssl_connection>(connection::shared_from_this());
    }

    private:
    /// Socket for the connection.
    ssl_socket socket_;
};

typedef boost::shared_ptr<ssl_connection> ssl_connection_ptr;

} // namespace server3
} // namespace http

#endif // SSL_CONNECTION
