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
#include <stdexcept>
#ifdef __linux__
#include <sys/sendfile.h>
#endif
#ifdef _apple_
#include <sys/uio.h>
#endif

using boost::asio::ip::tcp;

namespace http {
namespace server {
struct sendfile_op {
    public:
    typedef std::function<void(boost::system::error_code, std::size_t)> Handler;
    sendfile_op() = default;
    sendfile_op(tcp::socket *s, std::shared_ptr<file_descriptor> fd, Handler h) : sock_(s), fd(fd), handler(h) {
#ifdef __linux__
        struct stat st;
        if (::fstat(fd->fd, &st) != -1)
            file_len_ = st.st_size;
        else
            throw std::system_error{std::error_code{errno, std::system_category()}};
#endif
    }

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
#ifdef __linux__
                std::size_t count = file_len_ - offset;
#elif __APPLE__
                std::size_t count = 0;
#endif
                int n = this->native_sendfile(sock_->native_handle(), fd->value, count);
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
    std::shared_ptr<file_descriptor> fd;
#ifdef __linux__
    off64_t offset_;
    std::size_t file_len_;
#elif __APPLE__
    off_t offset_;
#endif
    std::size_t total_bytes_transferred_;
    Handler handler;

    private:
    int native_sendfile(int out_fd, int in_fd, std::size_t count) {
#ifdef __linux__
        return ::sendfile64(out_fd, in_fd, &offset_, count);
#elif __APPLE__
        (void)count;
        off_t len = 0;
        int ret = ::sendfile(in_fd, out_fd, offset_, &len, nullptr, 0);
        offset_ += len;
        return ret;

#endif
    }
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
