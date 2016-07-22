//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FILE_DESC
#define FILE_DESC
#include <boost/noncopyable.hpp>
#include <fcntl.h>
#include <string>
#include <unistd.h>

namespace http {
namespace server {

struct file_desc : private boost::noncopyable {
    int fd;

    file_desc() = default;
    file_desc(int fd) : fd(fd) {}
    file_desc(const std::string &path, int mode) : fd(::open(path.c_str(), mode)) {}
    ~file_desc() { ::close(fd); }

    operator int() const { return fd; }
};
}
}

#endif // FILE_DESC
