//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REPLY_HPP
#define HTTP_SERVER3_REPLY_HPP

#include "header.hpp"
#include "memory_mapping.hpp"
#include "sendfile_op.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace http {
namespace server {

namespace stock_replies {
static constexpr char ok[] = "";
static constexpr char created[] = "<html>"
                                  "<head><title>Created</title></head>"
                                  "<body><h1>201 Created</h1></body>"
                                  "</html>";
static constexpr char accepted[] = "<html>"
                                   "<head><title>Accepted</title></head>"
                                   "<body><h1>202 Accepted</h1></body>"
                                   "</html>";
static constexpr char no_content[] = "<html>"
                                     "<head><title>No Content</title></head>"
                                     "<body><h1>204 Content</h1></body>"
                                     "</html>";
static constexpr char multiple_choices[] = "<html>"
                                           "<head><title>Multiple Choices</title></head>"
                                           "<body><h1>300 Multiple Choices</h1></body>"
                                           "</html>";
static constexpr char moved_permanently[] = "<html>"
                                            "<head><title>Moved Permanently</title></head>"
                                            "<body><h1>301 Moved Permanently</h1></body>"
                                            "</html>";
static constexpr char moved_temporarily[] = "<html>"
                                            "<head><title>Moved Temporarily</title></head>"
                                            "<body><h1>302 Moved Temporarily</h1></body>"
                                            "</html>";
static constexpr char not_modified[] = "<html>"
                                       "<head><title>Not Modified</title></head>"
                                       "<body><h1>304 Not Modified</h1></body>"
                                       "</html>";
static constexpr char bad_request[] = "<html>"
                                      "<head><title>Bad Request</title></head>"
                                      "<body><h1>400 Bad Request</h1></body>"
                                      "</html>";
static constexpr char unauthorized[] = "<html>"
                                       "<head><title>Unauthorized</title></head>"
                                       "<body><h1>401 Unauthorized</h1></body>"
                                       "</html>";
static constexpr char forbidden[] = "<html>"
                                    "<head><title>Forbidden</title></head>"
                                    "<body><h1>403 Forbidden</h1></body>"
                                    "</html>";
static constexpr char not_found[] = "<html>"
                                    "<head><title>Not Found</title></head>"
                                    "<body><h1>404 Not Found</h1></body>"
                                    "</html>";
static constexpr char internal_server_error[] = "<html>"
                                                "<head><title>Internal Server Error</title></head>"
                                                "<body><h1>500 Internal Server Error</h1></body>"
                                                "</html>";
static constexpr char not_implemented[] = "<html>"
                                          "<head><title>Not Implemented</title></head>"
                                          "<body><h1>501 Not Implemented</h1></body>"
                                          "</html>";
static constexpr char bad_gateway[] = "<html>"
                                      "<head><title>Bad Gateway</title></head>"
                                      "<body><h1>502 Bad Gateway</h1></body>"
                                      "</html>";
static constexpr char service_unavailable[] = "<html>"
                                              "<head><title>Service Unavailable</title></head>"
                                              "<body><h1>503 Service Unavailable</h1></body>"
                                              "</html>";
}

namespace status_strings {
static constexpr char ok[] = "HTTP/1.1 200 OK\r\n";
static constexpr char created[] = "HTTP/1.1 201 Created\r\n";
static constexpr char accepted[] = "HTTP/1.1 202 Accepted\r\n";
static constexpr char no_content[] = "HTTP/1.1 204 No Content\r\n";
static constexpr char multiple_choices[] = "HTTP/1.1 300 Multiple Choices\r\n";
static constexpr char moved_permanently[] = "HTTP/1.1 301 Moved Permanently\r\n";
static constexpr char moved_temporarily[] = "HTTP/1.1 302 Moved Temporarily\r\n";
static constexpr char not_modified[] = "HTTP/1.1 304 Not Modified\r\n";
static constexpr char bad_request[] = "HTTP/1.1 400 Bad Request\r\n";
static constexpr char unauthorized[] = "HTTP/1.1 401 Unauthorized\r\n";
static constexpr char forbidden[] = "HTTP/1.1 403 Forbidden\r\n";
static constexpr char not_found[] = "HTTP/1.1 404 Not Found\r\n";
static constexpr char internal_server_error[] = "HTTP/1.1 500 Internal Server Errordddd\r\n";
static constexpr char not_implemented[] = "HTTP/1.1 501 Not Implemented\r\n";
static constexpr char bad_gateway[] = "HTTP/1.1 502 Bad Gateway\r\n";
static constexpr char service_unavailable[] = "HTTP/1.1 503 Service Unavailable\r\n";
}
namespace misc_strings {

static constexpr char name_value_separator[] = {':', ' '};
static constexpr char crlf[] = {'\r', '\n'};

} // namespace misc_strings

/// A reply to be sent to a client.
struct reply {
    /// The status of the reply.
    enum class status_type {
        undefined = 0,
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } status;

    reply();
    ~reply() = default;

    /// The headers to be included in the reply.
    std::vector<header> headers;

    /// The content to be sent in the reply.
    std::string content;

    sendfile_op sendfile;
    std::shared_ptr<char_memory_mapping> memory_mapping;

    /// Convert the reply into a vector of buffers. The buffers do not own the
    /// underlying memory blocks, therefore the reply object must remain valid and
    /// not be changed until the write operation has completed.
    std::vector<boost::asio::const_buffer> to_buffers();

    /// Checks to see if the response has a header set. Returns a pointer to
    /// the header object, or null if it doesn't exist
    header *get_header(const std::string &key);

    /// Sets a header if it already exists, or creates it
    /// FIXME: verify if the header exists, don't just push it
    void add_header(const std::string &key, const std::string &value);

    static std::string to_string(reply::status_type status);

    /// Get a stock reply.
    static reply stock_reply(status_type status);
    static boost::asio::const_buffer to_buffer(reply::status_type status);
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REPLY_HPP
