//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef USER_HANDLER_H
#define USER_HANDLER_H
#include "request.hpp"
#include <functional>
#include <regex>
#include <reply.hpp>

namespace http {
namespace server {

struct user_handler {
    public:
    typedef std::function<void(const request &, reply &)> handler;
    user_handler() = default;
    user_handler(const std::string &http_method, const std::regex &pattern, handler func)
        : http_method_(http_method), pattern_(pattern), handler_func_(func) {}

    /// Checks if a request matches this user handler. Verifies the HTTP method and tries to match the regex.
    bool matches(const request &req) const { return req.method == http_method_ && std::regex_match(req.uri, pattern_); }

    /// Simply invokes the user handler.
    void invoke(const request &req, reply &rep) const {
        handler_func_(std::forward<decltype(req)>(req), std::forward<decltype(rep)>(rep));
    }

    private:
    std::string http_method_;
    std::regex pattern_;
    handler handler_func_;
};
}
}

#endif // USER_HANDLER_H
