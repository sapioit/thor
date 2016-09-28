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
#include "log.hpp"
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

http::server::request_handler::request_handler(const std::string &doc_root, const std::string &compression_folder,
                                               const std::vector<http::server::user_handler> &user_handlers)
    : doc_root_(doc_root), compression_folder_(compression_folder), user_handlers_(user_handlers) {}

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
        if (!rep.get_header("Content-Encoding")) {
            handle_compression(req, rep);
        }
        auto header = rep.get_header("Connection");
        if (!header || uppercase(header->value) == "KEEP-ALIVE") {
            rep.add_header("Connection", "Keep-Alive");
        } else if (header && uppercase(header->value) == "CLOSE") {
            rep.add_header("Connection", "Close");
        }
    }
}

void http::server::request_handler::handle_compression(const http::server::request &req,
                                                       http::server::reply &rep) const {
    if (can_gzip(req)) {
        if (rep.content.size()) {
            // Compress using gzip
            std::stringstream compressed, original(rep.content);

            boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
            out.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip::best_compression));
            out.push(original);
            boost::iostreams::copy(out, compressed);

            rep.content = compressed.str();
        }
        rep.get_header("Content-Length")->value = std::to_string(rep.content.size());
        rep.add_header("Content-Encoding", "gzip");
    }
}

std::pair<bool, std::string>
http::server::request_handler::handle_compression_for_files(const http::server::request &req, http::server::reply &rep,
                                                            const std::string &full_uncompressed_path) const {
    if (can_gzip(req)) {
        auto file_name = req.uri.substr(req.uri.find_last_of('/') + 1);
        auto compressed_path = compression_folder_ + "/gzip." + file_name;
        auto temp_compressed_path = compressed_path + ".tmp";

        if (boost::filesystem::exists(compressed_path)) {
            rep.add_header("Content-Encoding", "gzip");
            return std::make_pair(true, compressed_path);
        }
        if (!boost::filesystem::exists(temp_compressed_path)) {
            auto pid = fork();
            if (pid == 0) {
                // Compress the file
                {
                    std::ifstream original(full_uncompressed_path);
                    std::ofstream compressed(temp_compressed_path);

                    boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
                    out.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip::best_compression));
                    out.push(original);

                    boost::iostreams::copy(out, compressed);
                }
                try {
                    // Rename the file atomically. If it fails, delete the old file
                    boost::filesystem::rename(temp_compressed_path, compressed_path);
                } catch (const boost::filesystem::filesystem_error &e) {
                    log::write("handle_compression_for_files: could not compress file " + full_uncompressed_path +
                               " exception message: " + e.what());
                    boost::system::error_code ec;
                    boost::filesystem::remove(temp_compressed_path, ec);
                    if (ec) {
                        log::write("handle_compression_for_files: could not remove the "
                                   "temporary compressed file");
                    }
                }
                std::exit(EXIT_SUCCESS);
            } else if (pid == -1) {
                log::write("handle_compression_for_files: fork failed");
            }
        }
    }

    // The file is being compressed at the moment or the client doesn't accept compression
    return std::make_pair(false, "");
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

bool http::server::request_handler::can_gzip(const http::server::request &req) {
    if (auto encoding_header = req.get_header("Accept-Encoding")) {
        if (encoding_header->value.find("gzip") != std::string::npos) {
            return true;
        }
    }
    return false;
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
