//
// file_descriptor.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "file_descriptor.hpp"
#include <fcntl.h>
#include <string>
#include <system_error>
#include <unistd.h>

http::server::file_descriptor::file_descriptor(const std::string &path, int mode)
    : value(::open(path.c_str(), mode)), path(path) {
    if (!good())
        throw std::system_error(std::error_code(errno, std::system_category()));
}

http::server::file_descriptor::~file_descriptor() { ::close(value); }
