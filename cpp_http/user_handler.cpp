//
// user_handler.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "user_handler.hpp"
http::server::uri_matchers::regex::regex() : matcher() {}

bool http::server::uri_matchers::regex::matches(const http::server::request &req) const {
    return req.method == method_ && std::regex_match(req.uri, pattern_);
}

http::server::uri_matchers::folder::folder() : matcher() {}

http::server::uri_matchers::folder::folder(const std::string &doc_root) : matcher(), doc_root_(doc_root) {}

bool http::server::uri_matchers::folder::matches(const http::server::request &req) const {
    bool method_ok = req.method == "GET";
    boost::filesystem::path full_path = doc_root_ + req.uri;
    bool is_folder = boost::filesystem::exists(full_path) && boost::filesystem::is_directory(full_path);
    return method_ok && is_folder;
}

http::server::user_handler::user_handler(std::unique_ptr<http::server::uri_matchers::matcher> matcher,
                                         http::server::user_handler::handler func)
    : matcher_(std::move(matcher)), handler_func_(func) {}

http::server::user_handler::user_handler(http::server::user_handler &&other) {
    if (this != &other) {
        *this = std::move(other);
    }
}

http::server::user_handler &http::server::user_handler::operator=(http::server::user_handler &&other) {
    matcher_ = std::move(other.matcher_);
    handler_func_ = std::move(other.handler_func_);
    return *this;
}

bool http::server::user_handler::matches(const http::server::request &req) const { return matcher_->matches(req); }

void http::server::user_handler::invoke(http::server::request &req, http::server::reply &rep) const {
    handler_func_(std::forward<decltype(req)>(req), std::forward<decltype(rep)>(rep));
}
