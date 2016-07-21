//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include "request_handler.hpp"
#include <boost/bind.hpp>
#include <vector>

namespace http {
namespace server {

connection::connection(boost::asio::io_service &io_service, request_handler &handler)
    : strand_(io_service), socket_(io_service), request_handler_(handler) {}

boost::asio::ip::tcp::socket &connection::socket() { return socket_; }

void connection::start() {
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            strand_.wrap(std::bind(&connection::handle_read, shared_from_this(), std::placeholders::_1,
                                                   std::placeholders::_2)));
}

void connection::handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) {
    if (!e) {
        boost::tribool result;
        boost::tie(result, boost::tuples::ignore) =
            request_parser_.parse(request_, buffer_.data(), buffer_.data() + bytes_transferred);

        if (result) {
            request_handler_.handle_request(request_, reply_, sendfile_);
            boost::asio::async_write(
                socket_, reply_.to_buffers(),
                strand_.wrap(std::bind(&connection::handle_write, shared_from_this(), std::placeholders::_1)));
        } else if (!result) {
            reply_ = reply::stock_reply(reply::bad_request);
            boost::asio::async_write(
                socket_, reply_.to_buffers(),
                strand_.wrap(std::bind(&connection::handle_write, shared_from_this(), std::placeholders::_1)));
        } else {
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    strand_.wrap(std::bind(&connection::handle_read, shared_from_this(),
                                                           std::placeholders::_1, std::placeholders::_2)));
        }
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the connection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The connection class's destructor closes the socket.
}

void connection::shutdown() {
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

void connection::handle_sendfile_done(const boost::system::error_code &e, std::size_t) {
    sendfile_ = {};
    if (!e)
        shutdown();
}

void connection::handle_write(const boost::system::error_code &e) {
    if (!e) {
        if (sendfile_) {
            sendfile_.sock_ = &socket_;
            sendfile_.handler = std::bind(&connection::handle_sendfile_done, shared_from_this(), std::placeholders::_1,
                                          std::placeholders::_2);
            socket_.async_write_some(boost::asio::null_buffers(), sendfile_);
        } else {
            // Initiate graceful connection closure.
            if (!e)
                shutdown();
        }
    }

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}

} // namespace server3
} // namespace http
