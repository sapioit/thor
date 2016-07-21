//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "user_handler.h"

http::server::user_handler::user_handler(const std::string &http_method, const std::regex &pattern, handler func)
    : http_method_(http_method), pattern_(pattern), handler_func_(func) {}

bool http::server::user_handler::matches(const http::server::request &req) const {
    return req.method == http_method_ && std::regex_match(req.uri, pattern_);
}

void http::server::user_handler::invoke(const http::server::request &req, http::server::reply &rep) const {
    handler_func_(std::forward<decltype(req)>(req), std::forward<decltype(rep)>(rep));
}
