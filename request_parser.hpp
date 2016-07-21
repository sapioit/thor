//
// request_parser.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_PARSER_HPP
#define HTTP_SERVER3_REQUEST_PARSER_HPP

#include "request.hpp"
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

namespace http {
namespace server {

/// Parser for incoming requests.
class request_parser {
    public:
    /// Construct ready to parse the request method.
    request_parser() : state_(method_start) {}

    /// Reset to initial parser state.
    void reset() { state_ = method_start; }

    /// Parse some data. The tribool return value is true when a complete request
    /// has been parsed, false if the data is invalid, indeterminate when more
    /// data is required. The InputIterator return value indicates how much of the
    /// input has been consumed.
    template <typename InputIterator>
    boost::tuple<boost::tribool, InputIterator> parse(request &req, InputIterator begin, InputIterator end) {
        while (begin != end) {
            boost::tribool result = consume(req, *begin++);
            if (result || !result)
                return boost::make_tuple(result, begin);
        }
        boost::tribool result = boost::indeterminate;
        return boost::make_tuple(result, begin);
    }

    private:
    /// Handle the next character of input.
    boost::tribool consume(request &req, char input) {
        switch (state_) {
        case method_start:
            if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return false;
            } else {
                state_ = method;
                req.method.push_back(input);
                return boost::indeterminate;
            }
        case method:
            if (input == ' ') {
                state_ = uri;
                return boost::indeterminate;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return false;
            } else {
                req.method.push_back(input);
                return boost::indeterminate;
            }
        case uri:
            if (input == ' ') {
                state_ = http_version_h;
                return boost::indeterminate;
            } else if (is_ctl(input)) {
                return false;
            } else {
                req.uri.push_back(input);
                return boost::indeterminate;
            }
        case http_version_h:
            if (input == 'H') {
                state_ = http_version_t_1;
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_t_1:
            if (input == 'T') {
                state_ = http_version_t_2;
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_t_2:
            if (input == 'T') {
                state_ = http_version_p;
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_p:
            if (input == 'P') {
                state_ = http_version_slash;
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_slash:
            if (input == '/') {
                req.http_version_major = 0;
                req.http_version_minor = 0;
                state_ = http_version_major_start;
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_major_start:
            if (is_digit(input)) {
                req.http_version_major = req.http_version_major * 10 + input - '0';
                state_ = http_version_major;
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_major:
            if (input == '.') {
                state_ = http_version_minor_start;
                return boost::indeterminate;
            } else if (is_digit(input)) {
                req.http_version_major = req.http_version_major * 10 + input - '0';
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_minor_start:
            if (is_digit(input)) {
                req.http_version_minor = req.http_version_minor * 10 + input - '0';
                state_ = http_version_minor;
                return boost::indeterminate;
            } else {
                return false;
            }
        case http_version_minor:
            if (input == '\r') {
                state_ = expecting_newline_1;
                return boost::indeterminate;
            } else if (is_digit(input)) {
                req.http_version_minor = req.http_version_minor * 10 + input - '0';
                return boost::indeterminate;
            } else {
                return false;
            }
        case expecting_newline_1:
            if (input == '\n') {
                state_ = header_line_start;
                return boost::indeterminate;
            } else {
                return false;
            }
        case header_line_start:
            if (input == '\r') {
                state_ = expecting_newline_3;
                return boost::indeterminate;
            } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
                state_ = header_lws;
                return boost::indeterminate;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return false;
            } else {
                req.headers.push_back(header());
                req.headers.back().name.push_back(input);
                state_ = header_name;
                return boost::indeterminate;
            }
        case header_lws:
            if (input == '\r') {
                state_ = expecting_newline_2;
                return boost::indeterminate;
            } else if (input == ' ' || input == '\t') {
                return boost::indeterminate;
            } else if (is_ctl(input)) {
                return false;
            } else {
                state_ = header_value;
                req.headers.back().value.push_back(input);
                return boost::indeterminate;
            }
        case header_name:
            if (input == ':') {
                state_ = space_before_header_value;
                return boost::indeterminate;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return false;
            } else {
                req.headers.back().name.push_back(input);
                return boost::indeterminate;
            }
        case space_before_header_value:
            if (input == ' ') {
                state_ = header_value;
                return boost::indeterminate;
            } else {
                return false;
            }
        case header_value:
            if (input == '\r') {
                state_ = expecting_newline_2;
                return boost::indeterminate;
            } else if (is_ctl(input)) {
                return false;
            } else {
                req.headers.back().value.push_back(input);
                return boost::indeterminate;
            }
        case expecting_newline_2:
            if (input == '\n') {
                state_ = header_line_start;
                return boost::indeterminate;
            } else {
                return false;
            }
        case expecting_newline_3:
            return (input == '\n');
        default:
            return false;
        }
    }

    /// Check if a byte is an HTTP character.
    static bool is_char(int c) { return c >= 0 && c <= 127; }

    /// Check if a byte is an HTTP control character.
    static bool is_ctl(int c) { return (c >= 0 && c <= 31) || (c == 127); }

    /// Check if a byte is defined as an HTTP tspecial character.
    static bool is_tspecial(int c) {
        switch (c) {
        case '(':
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case ':':
        case '\\':
        case '"':
        case '/':
        case '[':
        case ']':
        case '?':
        case '=':
        case '{':
        case '}':
        case ' ':
        case '\t':
            return true;
        default:
            return false;
        }
    }

    /// Check if a byte is a digit.
    static bool is_digit(int c) { return c >= '0' && c <= '9'; }

    /// The current state of the parser.
    enum state {
        method_start,
        method,
        uri,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3
    } state_;
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_PARSER_HPP
