//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_HPP
#define HTTP_SERVER3_REQUEST_HPP

#include "header.hpp"
#include <functional>
#include <string>
#include <vector>

namespace http {
namespace server {

/// A request received from a client.
struct request {
    std::string method;
    std::string uri;
    int http_version_major;
    int http_version_minor;
    std::vector<header> headers;
    std::string body;

    header *get_header(const std::string &key);

    const header *get_header(const std::string &key) const;
    const std::string &read_body();
    friend class connection;
    friend class ssl_connection;

    private:
    std::function<void(void)> read_body_func;
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HPP
