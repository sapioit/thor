//
// user_handler.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef USER_HANDLER_H
#define USER_HANDLER_H
#include "reply.hpp"
#include "request.hpp"
#include <functional>
#include <regex>

namespace http {
namespace server {

namespace uri_matchers {
class matcher {
    public:
    matcher() = default;
    virtual ~matcher() {}
    virtual bool matches(const request &req) const = 0;
};

class regex : public matcher {
    public:
    regex() : matcher() {}
    regex(const std::string &method, const std::regex &r) : method_(method), pattern_(r) {}
    bool matches(const request &req) const override {
        return req.method == method_ && std::regex_match(req.uri, pattern_);
    }

    private:
    std::string method_;
    std::regex pattern_;
};

class folder : public matcher {
    public:
    folder() : matcher() {}
    folder(const std::string &doc_root) : matcher(), doc_root_(doc_root) {}
    bool matches(const request &req) const override {
        bool method_ok = req.method == "GET";
        boost::filesystem::path full_path = doc_root_ + req.uri;
        bool is_folder = boost::filesystem::exists(full_path) && boost::filesystem::is_directory(full_path);
        return method_ok && is_folder;
    }

    private:
    std::string doc_root_;
};
}

struct user_handler {
    public:
    typedef std::function<void(request &, reply &)> handler;
    user_handler() = default;
    user_handler(std::unique_ptr<uri_matchers::matcher> matcher, handler func)
        : matcher_(std::move(matcher)), handler_func_(func) {}
    user_handler(const user_handler &) = delete;
    user_handler &operator=(const user_handler &) = delete;
    user_handler(user_handler &&other) {
        if (this != &other) {
            *this = std::move(other);
        }
    }
    user_handler &operator=(user_handler &&other) {
        matcher_ = std::move(other.matcher_);
        handler_func_ = std::move(other.handler_func_);
        return *this;
    }

    /// Checks if a request matches this user handler. Verifies the HTTP method and tries to match the regex.
    bool matches(const request &req) const { return matcher_->matches(req); }

    /// Simply invokes the user handler.
    void invoke(request &req, reply &rep) const {
        handler_func_(std::forward<decltype(req)>(req), std::forward<decltype(rep)>(rep));
    }

    private:
    std::unique_ptr<uri_matchers::matcher> matcher_;
    handler handler_func_;
};

typedef std::unique_ptr<uri_matchers::matcher> matcher_ptr;
}
}

#endif // USER_HANDLER_H
