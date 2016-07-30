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
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

using http::server::request;
using http::server::reply;
using http::server::user_handler;

void folder_1_trailing_slash(request &req, reply &rep) {
    try {
        rep.content = req.uri + "one trailing slash";
    } catch (const std::system_error &) {
        std::cout << "shit!" << std::endl;
    }
}
void folder_many_trailing_slashes(request &req, reply &rep) {
    try {
        rep.content = req.uri + "many trailing slashes";
    } catch (const std::system_error &) {
        std::cout << "shit!" << std::endl;
    }
}

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

        std::vector<user_handler> handlers{
            // Ending with one trailing slash
            {"GET", std::regex{"^[\\/]{0,1}[\\w]*[\\/]?$"},
             [argv](const auto &request, auto &reply) { list_directory(request, reply, argv[5]); }},
            // Ending with many trailing slashes
            {"GET", std::regex{"^[\\/]{0,1}[\\w]*[\\/]+$"}, folder_many_trailing_slashes}};

        // Initialise the server.
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[4]);
        http::server::server s(argv[1], argv[2], argv[3], argv[5], argv[6], num_threads, handlers);

        // Run the server until stopped.
        s.run();
    } catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}
