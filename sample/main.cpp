//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "directory_listing.hpp"
#include "server.hpp"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <iostream>
#include <string>

using http::server::request;
using http::server::reply;
using http::server::user_handler;
using http::server::matcher_ptr;
using namespace http::server::uri_matchers;
using namespace http::server;

int main(int argc, char *argv[]) {
    try {
        // Check command line arguments.
        if (argc != 7) {
            std::cerr << "Usage: http_server <address> <http_port> <https_port> <threads> <doc_root> <cert_root>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 80 443 1 . .\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 80 443 1 . .\n";
            return 1;
        }
        std::string address = argv[1];
        std::string http_port = argv[2];
        std::string https_port = argv[3];
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[4]);
        std::string doc_root = argv[5];
        std::string keys = argv[6];
        std::vector<user_handler> handlers;
        handlers.emplace_back(matcher_ptr(new folder{doc_root}),
                              std::bind(list_directory, std::placeholders::_1, std::placeholders::_2, doc_root));

        // Initialise the server.
        http::server::server s(address, http_port, https_port, doc_root, keys, num_threads, handlers);

        // Run the server until stopped.
        s.run();
    } catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}
