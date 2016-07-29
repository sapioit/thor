//
// file_descriptor.hpp
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

struct file_descriptor : private boost::noncopyable {
    int value;

    file_descriptor() = default;
    file_descriptor(int fd) : value(fd) {}
    file_descriptor(const std::string &path, int mode) : value(::open(path.c_str(), mode)) {}
    ~file_descriptor() { ::close(value); }

    bool good() const { return value != -1; }
};
}
}

#endif // FILE_DESC
