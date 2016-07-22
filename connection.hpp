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

#ifndef HTTP_SERVER3_CONNECTION_HPP
#define HTTP_SERVER3_CONNECTION_HPP

#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "sendfile_op.hpp"
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
namespace http {
namespace server {

/// Represents a single connection from a client.
class connection : public virtual boost::enable_shared_from_this<connection>, private boost::noncopyable {
    public:
    /// Construct a connection with the given io_service.
    explicit connection(boost::asio::io_service &io_service, request_handler &handler)
        : strand_(io_service), socket_(io_service), request_handler_(handler) {}

    virtual ~connection() {}

    /// Get the socket associated with the connection.
    virtual boost::asio::ip::tcp::socket &socket() { return socket_; }

    /// Start the first asynchronous operation for the connection.
    virtual void start() {
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            strand_.wrap(boost::bind(&connection::handle_read, shared_from_this(), boost::asio::placeholders::error,
                                     boost::asio::placeholders::bytes_transferred)));
    }

    virtual void shutdown() {
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    }

    protected:
    void handle_sendfile_done(const boost::system::error_code &, std::size_t) { sendfile_ = {}; }

    /// Handle completion of a read operation.
    virtual void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) {
        if (!e) {
            boost::tribool result;
            boost::tie(result, boost::tuples::ignore) =
                request_parser_.parse(request_, buffer_.data(), buffer_.data() + bytes_transferred);

            if (result) {
                request_handler_.handle_request<request_handler::protocol_type::http>(request_, reply_);
                if (reply_.sendfile)
                    sendfile_ = reply_.sendfile;
                boost::asio::async_write(socket_, reply_.to_buffers(),
                                         strand_.wrap(boost::bind(&connection::handle_write, shared_from_this(),
                                                                  boost::asio::placeholders::error)));
            } else if (!result) {
                reply_ = reply::stock_reply(reply::status_type::bad_request);
                boost::asio::async_write(socket_, reply_.to_buffers(),
                                         strand_.wrap(boost::bind(&connection::handle_write, shared_from_this(),
                                                                  boost::asio::placeholders::error)));
            } else {
                socket_.async_read_some(boost::asio::buffer(buffer_),
                                        strand_.wrap(boost::bind(&connection::handle_read, shared_from_this(),
                                                                 boost::asio::placeholders::error,
                                                                 boost::asio::placeholders::bytes_transferred)));
            }
        }

        // If an error occurs then no new asynchronous operations are started. This
        // means that all shared_ptr references to the connection object will
        // disappear and the object will be destroyed automatically after this
        // handler returns. The connection class's destructor closes the socket.
    }

    /// Handle completion of a write operation.
    virtual void handle_write(const boost::system::error_code &e) {
        if (!e) {
            if (sendfile_) {
                sendfile_.sock_ = &socket_;
                sendfile_.handler =
                    boost::bind(&connection::handle_sendfile_done, shared_from_this(), boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred);
                socket_.async_write_some(boost::asio::null_buffers(), sendfile_);
            }
        }

        // No new asynchronous operations are started. This means that all shared_ptr
        // references to the connection object will disappear and the object will be
        // destroyed automatically after this handler returns. The connection class's
        // destructor closes the socket.
    }

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_service::strand strand_;

    private:
    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    protected:
    /// The handler used to process the incoming request.
    request_handler &request_handler_;

    /// Buffer for incoming data.
    boost::array<char, 5 * 8192> buffer_;

    /// The incoming request.
    request request_;

    /// The parser for the incoming request.
    request_parser request_parser_;

    /// The reply to be sent back to the client.
    reply reply_;

    sendfile_op sendfile_;
};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP
