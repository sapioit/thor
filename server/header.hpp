//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_HEADER_HPP
#define HTTP_SERVER3_HEADER_HPP

#include <string>

namespace http {
namespace server {

struct header {
    std::string name;
    std::string value;
    header() = default;
    header(const std::string &name, const std::string &value);
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_HEADER_HPP
