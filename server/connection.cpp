//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

http::server::connection::connection(boost::asio::io_service &io_service, http::server::request_handler &handler)
    : strand_(io_service), socket_(io_service), request_handler_(handler) {}

http::server::connection::~connection() {}

tcp::socket &http::server::connection::socket() { return socket_; }

void http::server::connection::start() {
    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        strand_.wrap(boost::bind(&connection::handle_read, shared_from_this(), boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred)));
}

void http::server::connection::keep_alive() { start(); }

void http::server::connection::keep_alive_if_needed() {
    if (auto connection_field_ptr = reply_.get_header("Connection")) {
        if (uppercase(connection_field_ptr->value) == "KEEP-ALIVE") {
            request_ = {};
            reply_ = {};
            request_parser_ = {};
            keep_alive();
        }
    }
}

void http::server::connection::handle_sendfile_done(const boost::system::error_code &, std::size_t) {
    sendfile_ = {};
    keep_alive_if_needed();
}

void http::server::connection::drain_body(boost::system::error_code &ec) {
    auto content_len_ptr = request_.get_header("Content-Length");
    if (content_len_ptr) {
        auto content_length = boost::lexical_cast<std::size_t>(content_len_ptr->value);
        auto current_body_size = request_.body.size();
        auto left = content_length - current_body_size;
        request_.body.resize(left);
        sync_read(&request_.body.front(), request_.body.size(), ec);
    } else {
        throw std::logic_error{"Request doesn't have a body"};
    }
}

void http::server::connection::drain_body_if_needed() {
    if (request_.get_header("Content-Length") && !request_.body.size()) {
        boost::system::error_code ignored_ec;
        drain_body(ignored_ec);
    }
}

void http::server::connection::sync_read(char *where, std::size_t bytes, boost::system::error_code &ec) {
    boost::asio::read(socket_, boost::asio::buffer(where, bytes),
                      [bytes](const boost::system::error_code &, std::size_t bytes_read) -> std::size_t {
                          return bytes - bytes_read;
                      },
                      ec);
}

void http::server::connection::handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) {
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
            request_handler_.handle_request<request_handler::protocol_type::http>(request_, reply_);
            drain_body_if_needed();
            if (reply_.sendfile)
                sendfile_ = reply_.sendfile;
            boost::asio::async_write(socket_, reply_.to_buffers(),
                                     strand_.wrap(boost::bind(&connection::handle_write, shared_from_this(),
                                                              boost::asio::placeholders::error)));
        } else if (!result) {
            // The request is malformed.
            reply_ = reply::stock_reply(reply::status_type::bad_request);
            drain_body_if_needed();
            boost::asio::async_write(socket_, reply_.to_buffers(),
                                     strand_.wrap(boost::bind(&connection::handle_write, shared_from_this(),
                                                              boost::asio::placeholders::error)));
        } else {
            // Need more data.
            socket_.async_read_some(
                boost::asio::buffer(buffer_),
                strand_.wrap(boost::bind(&connection::handle_read, shared_from_this(), boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred)));
        }
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the connection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The connection class's destructor closes the socket.
}

void http::server::connection::handle_write(const boost::system::error_code &e) {
    if (!e) {
        if (sendfile_) {
            sendfile_.sock_ = &socket_;
            sendfile_.handler =
                boost::bind(&connection::handle_sendfile_done, shared_from_this(), boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred);
            socket_.async_write_some(boost::asio::null_buffers(), sendfile_);
        } else {
            keep_alive_if_needed();
        }
    }

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}
