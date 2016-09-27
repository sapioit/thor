//
// ssl_connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "ssl_connection.hpp"
http::server::ssl_connection::ssl_connection(boost::asio::io_service &io_service, boost::asio::ssl::context &context,
                                             http::server::request_handler &handler)
    : connection(io_service, handler), socket_(io_service, context) {}

http::server::ssl_connection::~ssl_connection() {}

void http::server::ssl_connection::start() {
    socket_.async_handshake(boost::asio::ssl::stream_base::server,
                            std::bind(&ssl_connection::start_reading, shared_from_this(), std::placeholders::_1));
}

void http::server::ssl_connection::start_reading(const boost::system::error_code &error) {
    if (!error) {
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            std::bind(&ssl_connection::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
}

void http::server::ssl_connection::sync_read(char *where, std::size_t bytes, boost::system::error_code &ec) {
    boost::asio::read(socket_, boost::asio::buffer(where, bytes),
                      [bytes](const boost::system::error_code &, std::size_t bytes_read) -> std::size_t {
                          return bytes - bytes_read;
                      },
                      ec);
}

void http::server::ssl_connection::handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) {
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
                std::bind(&ssl_connection::handle_write, shared_from_this(), std::placeholders::_1));
        } else if (!result) {
            // The request is malformed.
            reply_ = reply::stock_reply(reply::status_type::bad_request);
            drain_body_if_needed();
            boost::asio::async_write(
                socket_, reply_.to_buffers(),
                std::bind(&ssl_connection::handle_write, shared_from_this(), std::placeholders::_1));
        } else {
            // Need more data.
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    std::bind(&ssl_connection::handle_read, shared_from_this(), std::placeholders::_1,
                                              std::placeholders::_2));
        }
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the connection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The connection class's destructor closes the socket.
}

void http::server::ssl_connection::handle_write(const boost::system::error_code &e) {
    if (!e) {
        keep_alive_if_needed();
    } else {
        // print_err(e);
    }
    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}

void http::server::ssl_connection::handle_shutdown(const boost::system::error_code &) {}

void http::server::ssl_connection::print_err(boost::system::error_code error) {
    std::string err = error.message();
    if (error.category() == boost::asio::error::get_ssl_category()) {
        err = std::string(" (") + std::to_string(ERR_GET_LIB(error.value())) + "," +
              std::to_string(ERR_GET_FUNC(error.value())) + "," + std::to_string(ERR_GET_REASON(error.value())) + ") ";
        // ERR_PACK /* crypto/err/err.h */
        char buf[128];
        ::ERR_error_string_n(error.value(), buf, sizeof(buf));
        err += buf;
    }
    std::cout << err << std::endl;
}

boost::shared_ptr<http::server::ssl_connection> http::server::ssl_connection::shared_from_this() {
    return boost::dynamic_pointer_cast<ssl_connection>(connection::shared_from_this());
}
