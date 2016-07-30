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

#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include "char_memory_mapping_cache.hpp"
#include "file_descriptor_cache.hpp"
#include "memory_mapping.hpp"
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "sendfile_op.hpp"
#include "string_utils.hpp"
#include "user_handler.hpp"
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#include <fstream>
#include <string>

namespace http {
namespace server {

/// The common handler for all incoming requests.
class request_handler : private boost::noncopyable {
    public:
    enum protocol_type { http, https };

    /// Construct with a directory containing files to be served.
    explicit request_handler(const std::string &doc_root, const std::vector<user_handler> &user_handlers)
        : doc_root_(doc_root), user_handlers_(user_handlers) {}

    /// Handle a request and produce a reply.
    template <protocol_type protocol> void handle_request(request &req, reply &rep) const {
        user_handler u_handler;
        if (has_user_handler(req, u_handler))
            invoke_user_handler(req, rep, u_handler);
        else
            handle_request_internally<protocol>(req, rep);
    }

    private:
    /// The directory containing the files to be served.
    std::string doc_root_;
    const std::vector<user_handler> &user_handlers_;

    /// Checks all the user handlers and returns false if there is none or true if there is. Also, if it
    /// return strue, the second argument will contain the user handler
    bool has_user_handler(const request &req, user_handler &handler) const {
        auto it = std::find_if(user_handlers_.begin(), user_handlers_.end(),
                               [&req](const user_handler &u_handler) { return u_handler.matches(req); });
        if (it != user_handlers_.end()) {
            handler = *it;
            return true;
        }
        return false;
    }

    /// Processes the request and returns either a stock resposne or a file
    template <protocol_type> void add_file(reply &rep, const std::string &full_path) const;

    template <protocol_type protocol> void handle_request_internally(const request &req, reply &rep) const {
        // Decode url to path.
        std::string request_path;
        if (!url_decode(req.uri, request_path)) {
            rep = reply::stock_reply(reply::status_type::bad_request);
            return;
        }

        // Request path must be absolute and not contain "..".
        if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos) {
            rep = reply::stock_reply(reply::status_type::bad_request);
            return;
        }

        // If path ends in slash (i.e. is a directory) then add "index.html".
        if (request_path[request_path.size() - 1] == '/') {
            request_path += "index.html";
        }

        // Open the file to send back.
        std::string full_path = doc_root_ + request_path;
        if (!boost::filesystem::exists(full_path)) {
            rep = reply::stock_reply(reply::status_type::not_found);
            return;
        }

        add_file<protocol>(rep, full_path);

        if (rep.status == reply::status_type::ok) {
            /// Statuses other than OK shouldn't have the fields set in this scope
            rep.add_header("Content-Length", std::to_string(boost::filesystem::file_size(full_path)));
            rep.add_header("Content-Type", mime_types::get_mime_type(full_path));
            auto header = req.get_header("Connection");
            if (!header || uppercase(header->value) == "KEEP-ALIVE") {
                rep.add_header("Connection", "Keep-Alive");
            } else if (header && uppercase(header->value) == "CLOSE") {
                rep.add_header("Connection", "Close");
            }
        }
    }

    /// Invokes the user handler and fixes the missing headers
    void invoke_user_handler(request &req, reply &rep, const user_handler &u_handler) const {
        u_handler.invoke(req, rep);

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

    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool url_decode(const std::string &in, std::string &out) {
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
};

template <>
void request_handler::add_file<request_handler::protocol_type::http>(reply &rep, const std::string &full_path) const {
    try {
        rep.sendfile.fd = file_descriptor_cache::get(full_path, O_RDONLY);
        rep.status = reply::status_type::ok;
    } catch (const std::logic_error &) {
        rep = reply::stock_reply(reply::status_type::not_found);
        return;
    }
}

template <>
void request_handler::add_file<request_handler::protocol_type::https>(reply &rep, const std::string &full_path) const {
    try {
        rep.memory_mapping = char_memory_mapping_cache::get(full_path, O_RDONLY);
        // Fill out the reply to be sent to the client.
        rep.status = reply::status_type::ok;
    } catch (const std::system_error &) {
        rep = reply::stock_reply(reply::status_type::not_found);
        return;
    }
}

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP
