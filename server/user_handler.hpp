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
    regex();
    regex(const std::string &method, const std::regex &r) : method_(method), pattern_(r) {}
    bool matches(const request &req) const override;

    private:
    std::string method_;
    std::regex pattern_;
};

class folder : public matcher {
    public:
    folder();
    folder(const std::string &doc_root);
    bool matches(const request &req) const override;

    private:
    std::string doc_root_;
};
}

struct user_handler {
    public:
    typedef std::function<void(request &, reply &)> handler;
    user_handler() = default;
    user_handler(std::unique_ptr<uri_matchers::matcher> matcher, handler func);
    user_handler(const user_handler &) = delete;
    user_handler &operator=(const user_handler &) = delete;
    user_handler(user_handler &&other);
    user_handler &operator=(user_handler &&other);

    /// Checks if a request matches this user handler. Verifies the HTTP method and tries to match the regex.
    bool matches(const request &req) const;

    /// Simply invokes the user handler.
    void invoke(request &req, reply &rep) const;

    private:
    std::unique_ptr<uri_matchers::matcher> matcher_;
    handler handler_func_;
};

typedef std::unique_ptr<uri_matchers::matcher> matcher_ptr;
}
}

#endif // USER_HANDLER_H
