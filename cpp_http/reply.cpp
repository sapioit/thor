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

#include "reply.hpp"

http::server::reply::reply() : status(status_type::undefined) {}

std::vector<boost::asio::const_buffer> http::server::reply::to_buffers() {
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
        buffers.push_back(boost::asio::const_buffer(&memory_mapping->at(0), memory_mapping->size()));
    else
        buffers.push_back(boost::asio::buffer(content));

    return buffers;
}

http::server::header *http::server::reply::get_header(const std::string &key) {
    auto it = std::find_if(headers.begin(), headers.end(), [&key](const header &h) { return h.name == key; });
    return it != headers.end() ? &*it : nullptr;
}

void http::server::reply::add_header(const std::string &key, const std::string &value) {
    headers.emplace_back(key, value);
}

std::string http::server::reply::to_string(http::server::reply::status_type status) {
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

http::server::reply http::server::reply::stock_reply(http::server::reply::status_type status) {
    reply rep;
    rep.status = status;
    rep.content = to_string(status);
    rep.add_header("Content-Length", std::to_string(rep.content.size()));
    rep.add_header("Content-Type", "text/html");
    rep.add_header("Connection", "Close");
    return rep;
}

boost::asio::const_buffer http::server::reply::to_buffer(http::server::reply::status_type status) {
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
