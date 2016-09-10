//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"

http::server::request_handler::request_handler(const std::string &doc_root,
                                               const std::vector<http::server::user_handler> &user_handlers)
    : doc_root_(doc_root), user_handlers_(user_handlers) {}

const http::server::user_handler *
http::server::request_handler::get_user_handler(const http::server::request &req) const {
    auto it = std::find_if(user_handlers_.cbegin(), user_handlers_.cend(),
                           [&req](const user_handler &u_handler) { return u_handler.matches(req); });
    if (it != user_handlers_.cend()) {
        return &*it;
    }
    return nullptr;
}

void http::server::request_handler::invoke_user_handler(http::server::request &req, http::server::reply &rep,
                                                        const http::server::user_handler *u_handler) const {
    u_handler->invoke(req, rep);

    if (rep.status == reply::status_type::undefined)
        rep.status = reply::status_type::ok;

    /// Set the necessary fields that haven't been set by the user handler
    if (rep.status == reply::status_type::ok) {
        /// Statuses other than OK shouldn't have the fields set in this scope
        if (!rep.get_header("Content-Length")) {
            rep.add_header("Content-Length", std::to_string(rep.content.size()));
        }
        if (!rep.get_header("Content-Type")) {
            rep.add_header("Content-Type", "text/plain");
        }
        auto header = rep.get_header("Connection");
        if (!header || uppercase(header->value) == "KEEP-ALIVE") {
            rep.add_header("Connection", "Keep-Alive");
        } else if (header && uppercase(header->value) == "CLOSE") {
            rep.add_header("Connection", "Close");
        }
    }
}

bool http::server::request_handler::url_decode(const std::string &in, std::string &out) {
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 3 <= in.size()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return true;
}

namespace http {
namespace server {
template <>
void http::server::request_handler::add_file<http::server::request_handler::protocol_type::http>(
    reply &rep, const std::string &full_path) const {
    try {
        rep.sendfile.fd = file_descriptor_cache::get(full_path, O_RDONLY);
        rep.status = reply::status_type::ok;
    } catch (const std::logic_error &) {
        rep = reply::stock_reply(reply::status_type::not_found);
        return;
    }
}

template <>
void http::server::request_handler::add_file<http::server::request_handler::protocol_type::https>(
    reply &rep, const std::string &full_path) const {
    try {
        rep.memory_mapping = char_memory_mapping_cache::get(full_path, O_RDONLY);
        // Fill out the reply to be sent to the client.
        rep.status = reply::status_type::ok;
    } catch (const std::system_error &) {
        rep = reply::stock_reply(reply::status_type::not_found);
        return;
    }
}
}
}
