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
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
namespace http {
namespace server {

/// Represents a single connection from a client.
class connection : public virtual boost::enable_shared_from_this<connection>, private boost::noncopyable {
    public:
    /// Construct a connection with the given io_service.
    explicit connection(boost::asio::io_service &io_service, request_handler &handler);

    virtual ~connection();

    /// Get the socket associated with the connection.
    virtual boost::asio::ip::tcp::socket &socket();

    /// Start the first asynchronous operation for the connection.
    virtual void start();

    protected:
    virtual void keep_alive();

    void keep_alive_if_needed();

    void handle_sendfile_done(const boost::system::error_code &, std::size_t);

    void drain_body(boost::system::error_code &ec);

    void drain_body_if_needed();

    virtual void sync_read(char *where, std::size_t bytes, boost::system::error_code &ec);

    /// Handle completion of a read operation.
    virtual void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred);

    /// Handle completion of a write operation.
    virtual void handle_write(const boost::system::error_code &e);

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_service::strand strand_;

    private:
    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    protected:
    /// The handler used to process the incoming request.
    request_handler &request_handler_;

    /// Buffer for incoming data.
    boost::array<char, 8192> buffer_;

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
