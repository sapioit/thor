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
    user_handler(const std::string &http_method, const std::regex &pattern, handler func);

    /// Checks if a request matches this user handler. Verifies the HTTP method and tries to match the regex.
    bool matches(const request &req) const;

    /// Simply invokes the user handler.
    void invoke(const request &req, reply &rep) const;

    private:
    std::string http_method;
    std::regex pattern;
    handler handler_func;
};
}
}

#endif // USER_HANDLER_H
