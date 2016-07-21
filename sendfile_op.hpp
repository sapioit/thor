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

#include "file_desc.h"
#include <boost/asio.hpp>
#include <sys/sendfile.h>

using boost::asio::ip::tcp;

namespace http {
namespace server {
struct sendfile_op {
    public:
    typedef std::function<void(boost::system::error_code, std::size_t)> Handler;
    sendfile_op() = default;
    sendfile_op(tcp::socket *s, std::shared_ptr<file_desc> fd, Handler h) : sock_(s), fd(fd), handler(h) {}

    // Function call operator meeting WriteHandler requirements.
    // Used as the handler for the async_write_some operation.
    void operator()(boost::system::error_code ec, std::size_t) {
        assert(handler && sock_ && fd);
        // Put the underlying socket into non-blocking mode.
        if (!ec)
            if (!sock_->native_non_blocking())
                sock_->native_non_blocking(true, ec);

        if (!ec) {
            for (;;) {
                // Try the system call.
                errno = 0;
                int n = native_sendfile(sock_->native_handle(), fd.get()->fd, 65536);
                ec = boost::system::error_code(n < 0 ? errno : 0, boost::asio::error::get_system_category());
                total_bytes_transferred_ += ec ? 0 : n;

                // Retry operation immediately if interrupted by signal.
                if (ec == boost::asio::error::interrupted)
                    continue;

                // Check if we need to run the operation again.
                if (ec == boost::asio::error::would_block || ec == boost::asio::error::try_again) {
                    // We have to wait for the socket to become ready again.
                    sock_->async_write_some(boost::asio::null_buffers(), *this);
                    return;
                }

                if (ec || n == 0) {
                    // An error occurred, or we have reached the end of the file.
                    // Either way we must exit the loop so we can call the handler.
                    break;
                }

                // Loop around to try calling sendfile again.
            }
        }

        // Pass result back to user's handler.
        handler(ec, total_bytes_transferred_);
    }

    operator bool() const { return fd.get(); }

    public:
    tcp::socket *sock_;
    std::shared_ptr<file_desc> fd;
    off64_t offset_;
    std::size_t total_bytes_transferred_;
    Handler handler;

    private:
#ifdef __linux__
    int native_sendfile(int out_fd, int in_fd, std::size_t count) {
        return ::sendfile64(out_fd, in_fd, &offset_, count);
    }
#endif
#ifdef __apple_
    int native_sendfile(int out_fd, int in_fd, off_t *offset, std::size_t count) {
        return 0;
        /// TODO fill out
    }
#endif
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
