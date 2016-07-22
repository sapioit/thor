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

#include "file_desc_cache.h"
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "sendfile_op.hpp"
#include "user_handler.hpp"
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#include <string>

namespace http {
namespace server {

/// The common handler for all incoming requests.
class request_handler : private boost::noncopyable {
    public:
    /// Construct with a directory containing files to be served.
    explicit request_handler(const std::string &doc_root, std::vector<user_handler> user_handlers)
        : doc_root_(doc_root), user_handlers_(user_handlers) {}

    /// Handle a request and produce a reply.
    void handle_request(const request &req, reply &rep, sendfile_op &sendfile) const {
        user_handler u_handler;
        if (has_user_handler(req, u_handler))
            invoke_user_handler(req, rep, u_handler);
        else
            handle_request_internally(req, rep, sendfile);
    }

    private:
    /// The directory containing the files to be served.
    std::string doc_root_;
    std::vector<user_handler> user_handlers_;

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
    void handle_request_internally(const request &req, reply &rep, sendfile_op &sendfile) const {
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
        rep.status = reply::status_type::ok;
        sendfile.fd = file_desc_cache::get(full_path, O_RDONLY);

        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(boost::filesystem::file_size(full_path));
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = mime_types::get_mime_type(full_path);

        /*std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
      if (!is) {
          rep = reply::stock_reply(reply::not_found);
          return;
      }

      // Fill out the reply to be sent to the client.
      rep.status = reply::ok;
      char buf[512];
      while (is.read(buf, sizeof(buf)).gcount() > 0)
          rep.content.append(buf, is.gcount());
      rep.headers.resize(2);
      rep.headers[0].name = "Content-Length";
      rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
      rep.headers[1].name = "Content-Type";
      rep.headers[1].value = mime_types::extension_to_type(extension);*/
    }

    /// Invokes the user handler and fixes the missing headers
    void invoke_user_handler(const request &req, reply &rep, const user_handler &u_handler) const {
        u_handler.invoke(req, rep);

        if (rep.status == reply::status_type::undefined)
            rep.status = reply::status_type::ok;

        {
            std::string value;
            if (!rep.has_header("Content-Length", value) || value == "") {
                rep.set_or_add_header("Content-Length", std::to_string(rep.content.size()));
            }
        }
        {
            std::string value;
            if (!rep.has_header("Content-Type", value) || value == "") {
                rep.set_or_add_header("Content-Type", "text/plain");
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

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP
