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

#include "sendfile_op.hpp"
#include <boost/filesystem.hpp>
#include <stdexcept>
#ifdef __linux__
#include <sys/sendfile.h>
#endif
#ifdef _apple_
#include <sys/uio.h>
#endif
http::server::sendfile_op::sendfile_op() : offset_(0), file_len_(0), total_bytes_transferred_(0) {}

http::server::sendfile_op::sendfile_op(tcp::socket *s, std::shared_ptr<http::server::file_descriptor> fd,
                                       http::server::sendfile_op::Handler h)
    : sock_(s), fd(fd), handler(h), offset_(0), file_len_(0), total_bytes_transferred_(0) {}

void http::server::sendfile_op::operator()(boost::system::error_code ec, std::size_t) {
    assert(handler && sock_ && fd);
#ifdef __linux__
    if (!file_len_) {
        file_len_ = boost::filesystem::file_size(fd->path);
    }
#endif
    // Put the underlying socket into non-blocking mode.
    if (!ec)
        if (!sock_->native_non_blocking())
            sock_->native_non_blocking(true, ec);

    if (!ec) {
        for (; offset_ <= file_len_;) {
            // Try the system call.
            errno = 0;
#ifdef __linux__
            auto count = file_len_ - offset_;
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

            if (ec || n == 0 || n == count) {
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

int http::server::sendfile_op::native_sendfile(int out_fd, int in_fd, std::size_t count) {
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

http::server::sendfile_op::operator bool() const { return fd.get(); }
