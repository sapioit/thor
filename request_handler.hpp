//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include "user_handler.h"

namespace http {
namespace server {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
  : private boost::noncopyable
{
public:
  /// Construct with a directory containing files to be served.
  explicit request_handler(const std::string& doc_root, std::vector<user_handler> user_handlers_);

  /// Handle a request and produce a reply.
  void handle_request(const request& req, reply& rep) const;

private:
  /// The directory containing the files to be served.
  std::string doc_root_;
  std::vector<user_handler> user_handlers_;

  /// Checks all the user handlers and returns false if there is none or true if there is. Also, if it
  /// return strue, the second argument will contain the user handler
  bool has_user_handler(const request&, user_handler&) const;

  /// Processes the request and returns either a stock resposne or a file
  void handle_request_internally(const request& req, reply& rep) const;

  /// Invokes the user handler and fixes the missing headers
  void invoke_user_handler(const request& req, reply& rep, const user_handler& u_handler) const;

  /// Perform URL-decoding on a string. Returns false if the encoding was
  /// invalid.
  static bool url_decode(const std::string& in, std::string& out);
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP
