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
static constexpr char ok[] = "HTTP/1.0 200 OK\r\n";
static constexpr char created[] = "HTTP/1.0 201 Created\r\n";
static constexpr char accepted[] = "HTTP/1.0 202 Accepted\r\n";
static constexpr char no_content[] = "HTTP/1.0 204 No Content\r\n";
static constexpr char multiple_choices[] = "HTTP/1.0 300 Multiple Choices\r\n";
static constexpr char moved_permanently[] = "HTTP/1.0 301 Moved Permanently\r\n";
static constexpr char moved_temporarily[] = "HTTP/1.0 302 Moved Temporarily\r\n";
static constexpr char not_modified[] = "HTTP/1.0 304 Not Modified\r\n";
static constexpr char bad_request[] = "HTTP/1.0 400 Bad Request\r\n";
static constexpr char unauthorized[] = "HTTP/1.0 401 Unauthorized\r\n";
static constexpr char forbidden[] = "HTTP/1.0 403 Forbidden\r\n";
static constexpr char not_found[] = "HTTP/1.0 404 Not Found\r\n";
static constexpr char internal_server_error[] = "HTTP/1.0 500 Internal Server Errordddd\r\n";
static constexpr char not_implemented[] = "HTTP/1.0 501 Not Implemented\r\n";
static constexpr char bad_gateway[] = "HTTP/1.0 502 Bad Gateway\r\n";
static constexpr char service_unavailable[] = "HTTP/1.0 503 Service Unavailable\r\n";
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

    reply() : status(status_type::undefined) {}
    ~reply() {}

    /// The headers to be included in the reply.
    std::vector<header> headers;

    /// The content to be sent in the reply.
    std::string content;

    sendfile_op sendfile;
    std::shared_ptr<char_memory_mapping> memory_mapping;

    /// Convert the reply into a vector of buffers. The buffers do not own the
    /// underlying memory blocks, therefore the reply object must remain valid and
    /// not be changed until the write operation has completed.
    std::vector<boost::asio::const_buffer> to_buffers() {
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(to_buffer(status));
        for (std::size_t i = 0; i < headers.size(); ++i) {
            header &h = headers[i];
            buffers.push_back(boost::asio::buffer(h.name));
            buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
            buffers.push_back(boost::asio::buffer(h.value));
            buffers.push_back(boost::asio::buffer(misc_strings::crlf));
        }
        buffers.push_back(boost::asio::buffer(misc_strings::crlf));
        if (memory_mapping)
            buffers.push_back(boost::asio::buffer(&memory_mapping->at(0), memory_mapping->size()));
        else
            buffers.push_back(boost::asio::buffer(content));

        return buffers;
    }

    /// Checks to see if the response has a header set. Return false if it doesn't,
    /// or true if it has and sets the second argument to its value.
    bool has_header(const std::string &key, std::string &value) const {
        auto it = std::find_if(headers.begin(), headers.end(), [&key](const header &h) { return h.name == key; });
        if (it != headers.end()) {
            value = it->value;
            return true;
        }
        return false;
    }

    /// Sets a header if it already exists, or creates it
    /// FIXME: verify if the header exists, don't just push it
    void set_or_add_header(const std::string &key, const std::string &value) { headers.emplace_back(key, value); }

    static std::string to_string(reply::status_type status) {
        switch (status) {
        case reply::status_type::ok:
            return stock_replies::ok;
        case reply::status_type::created:
            return stock_replies::created;
        case reply::status_type::accepted:
            return stock_replies::accepted;
        case reply::status_type::no_content:
            return stock_replies::no_content;
        case reply::status_type::multiple_choices:
            return stock_replies::multiple_choices;
        case reply::status_type::moved_permanently:
            return stock_replies::moved_permanently;
        case reply::status_type::moved_temporarily:
            return stock_replies::moved_temporarily;
        case reply::status_type::not_modified:
            return stock_replies::not_modified;
        case reply::status_type::bad_request:
            return stock_replies::bad_request;
        case reply::status_type::unauthorized:
            return stock_replies::unauthorized;
        case reply::status_type::forbidden:
            return stock_replies::forbidden;
        case reply::status_type::not_found:
            return stock_replies::not_found;
        case reply::status_type::internal_server_error:
            return stock_replies::internal_server_error;
        case reply::status_type::not_implemented:
            return stock_replies::not_implemented;
        case reply::status_type::bad_gateway:
            return stock_replies::bad_gateway;
        case reply::status_type::service_unavailable:
            return stock_replies::service_unavailable;
        default:
            return stock_replies::internal_server_error;
        }
    }

    /// Get a stock reply.
    static reply stock_reply(status_type status) {
        reply rep;
        rep.status = status;
        rep.content = to_string(status);
        rep.set_or_add_header("Content-Length", std::to_string(rep.content.size()));
        rep.set_or_add_header("Content-Type", "text/html");
        return rep;
    }
    static boost::asio::const_buffer to_buffer(reply::status_type status) {
        switch (status) {
        case reply::status_type::ok:
            return boost::asio::buffer(status_strings::ok);
        case reply::status_type::created:
            return boost::asio::buffer(status_strings::created);
        case reply::status_type::accepted:
            return boost::asio::buffer(status_strings::accepted);
        case reply::status_type::no_content:
            return boost::asio::buffer(status_strings::no_content);
        case reply::status_type::multiple_choices:
            return boost::asio::buffer(status_strings::multiple_choices);
        case reply::status_type::moved_permanently:
            return boost::asio::buffer(status_strings::moved_permanently);
        case reply::status_type::moved_temporarily:
            return boost::asio::buffer(status_strings::moved_temporarily);
        case reply::status_type::not_modified:
            return boost::asio::buffer(status_strings::not_modified);
        case reply::status_type::bad_request:
            return boost::asio::buffer(status_strings::bad_request);
        case reply::status_type::unauthorized:
            return boost::asio::buffer(status_strings::unauthorized);
        case reply::status_type::forbidden:
            return boost::asio::buffer(status_strings::forbidden);
        case reply::status_type::not_found:
            return boost::asio::buffer(status_strings::not_found);
        case reply::status_type::internal_server_error:
            return boost::asio::buffer(status_strings::internal_server_error);
        case reply::status_type::not_implemented:
            return boost::asio::buffer(status_strings::not_implemented);
        case reply::status_type::bad_gateway:
            return boost::asio::buffer(status_strings::bad_gateway);
        case reply::status_type::service_unavailable:
            return boost::asio::buffer(status_strings::service_unavailable);
        default:
            return boost::asio::buffer(status_strings::internal_server_error);
        }
    }
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REPLY_HPP
