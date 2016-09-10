//
// sendfile_op.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SENDFILE_OP_H
#define SENDFILE_OP_H

#include "file_descriptor.hpp"
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

namespace http {
namespace server {
struct sendfile_op {
    public:
    typedef std::function<void(boost::system::error_code, std::size_t)> Handler;
    sendfile_op();
    sendfile_op(tcp::socket *s, std::shared_ptr<file_descriptor> fd, Handler h);

    // Function call operator meeting WriteHandler requirements.
    // Used as the handler for the async_write_some operation.
    void operator()(boost::system::error_code ec, std::size_t);

    operator bool() const;

    public:
    tcp::socket *sock_;
    std::shared_ptr<file_descriptor> fd;
    Handler handler;
#ifdef __linux__
    off64_t offset_;
    off64_t file_len_;
#elif __APPLE__
    off_t offset_;
#endif
    std::size_t total_bytes_transferred_;

    private:
    int native_sendfile(int out_fd, int in_fd, std::size_t count);
};
}
}

/*
 * template <typename Handler>
 * void async_sendfile(tcp::socket& sock, int fd, Handler h)
 * {
 * sendfile_op<Handler> op = { sock, fd, h, 0, 0 };
 * sock.async_write_some(boost::asio::null_buffers(), op);
 * }
 */

#endif // SENDFILE_OP_H
