//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request.hpp"
#include <algorithm>

http::server::header *http::server::request::get_header(const std::string &key) {
    auto it = std::find_if(headers.begin(), headers.end(), [&key](const header &h) { return h.name == key; });
    return it != headers.end() ? &*it : nullptr;
}

const http::server::header *http::server::request::get_header(const std::string &key) const {
    auto it = std::find_if(headers.cbegin(), headers.cend(), [&key](const header &h) { return h.name == key; });
    return it != headers.end() ? &*it : nullptr;
}

const std::string &http::server::request::read_body() {
    try {
        read_body_func();
        return body;
    } catch (...) {
        throw;
    }
}
