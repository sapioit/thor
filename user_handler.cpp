#include "user_handler.h"

http::server::user_handler::user_handler(const std::string &http_method, const std::regex &pattern, handler func)
    : http_method(http_method), pattern(pattern), handler_func(func) {}

bool http::server::user_handler::matches(const http::server::request &req) const {
    return req.method == http_method && std::regex_match(req.uri, pattern);
}

void http::server::user_handler::invoke(const http::server::request &req, http::server::reply &rep) const {
    handler_func(std::forward<decltype(req)>(req), std::forward<decltype(rep)>(rep));
}
