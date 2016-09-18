//
// directory_listing.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "directory_listing.hpp"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <iomanip>
#include <regex>
#include <sstream>

namespace {
std::string trim_quotes(std::string str) {
    if (str.front() == '\"')
        str = str.substr(1, str.size());
    if (str.back() == '\"')
        str = str.substr(0, str.size());
    return str;
}

std::string make_link(const std::string &uri, const boost::filesystem::path &p) {
    std::string link = uri;
    if (uri.back() != '/')
        link += '/';
    link += p.filename().string();
    if (boost::filesystem::is_directory(p))
        link += '/';

    return link;
}

std::string make_file_name(const boost::filesystem::path &p) {
    if (boost::filesystem::is_directory(p)) {
        return p.filename().string() + "/";
    } else if (boost::filesystem::is_regular_file(p)) {
        return p.filename().string();
    }
    throw std::logic_error{"Trying to link a special file"};
}

boost::filesystem::path relative_to(boost::filesystem::path from, boost::filesystem::path to) {
    // Start at the root path and while they are the same then do nothing then when they first
    // diverge take the remainder of the two path and replace the entire from path with ".."
    // segments.
    boost::filesystem::path::const_iterator from_iter = from.begin();
    boost::filesystem::path::const_iterator to_iter = to.begin();

    // Loop through both
    while (from_iter != from.end() && to_iter != to.end() && (*to_iter) == (*from_iter)) {
        ++to_iter;
        ++from_iter;
    }

    boost::filesystem::path final_path;
    while (from_iter != from.end()) {
        final_path /= "..";
        ++from_iter;
    }

    while (to_iter != to.end()) {
        final_path /= *to_iter;
        ++to_iter;
    }

    return final_path;
}

std::string parent_directory_anchor(std::string uri, std::string root_path) {
    if (std::all_of(uri.begin(), uri.end(), [](char c) { return c == '/'; })) {
        return "";
    }
    if (uri.back() != '/')
        uri += '/';
    if (root_path.back() == '/')
        root_path = root_path.substr(0, root_path.length() - 1);
    boost::filesystem::path p(root_path + uri);
    boost::filesystem::path parent = p.parent_path().parent_path();
    boost::filesystem::path relative = relative_to(parent, root_path);

    std::string link = '/' + relative.string();
    if (relative.string() != "")
        link += "/";

    std::ostringstream stream;
    stream << "<a href=\"" << link << "\">"
           << ".."
           << "</a><br/>";

    return stream.str();
}
}

void http::server::list_directory(const http::server::request &req, http::server::reply &reply,
                                  const std::string &doc_root) {
    try {
        boost::filesystem::path root = doc_root + req.uri;
        std::ostringstream stream;
        stream << "<h1>Directory listing of " + req.uri + "</h1>";
        stream << parent_directory_anchor(req.uri, doc_root);

        std::vector<boost::filesystem::path> files_in_folder;

        for (auto it : boost::filesystem::directory_iterator(root)) {
            files_in_folder.push_back(it);
        }
        std::sort(files_in_folder.begin(), files_in_folder.end(),
                  [](const boost::filesystem::path &p1, const boost::filesystem::path &p2) {
                      return boost::filesystem::last_write_time(p1) > boost::filesystem::last_write_time(p2);
                  });
        for (auto it : files_in_folder) {
            try {
                boost::filesystem::path p = it;
                stream << "<a href=\"";
                stream << make_link(req.uri, p) << "\">";
                stream << trim_quotes(make_file_name(p));
                stream << "</a><br/>";
            } catch (const std::logic_error &) {
                continue;
            }
        }
        reply.content = stream.str();
        reply.add_header("Cache-Control", "max-age=60");
        reply.add_header("Content-Type", "text/html");
    } catch (...) {
        reply.status = reply::status_type::internal_server_error;
        reply.add_header("Cache-Control", "no-cache");
    }
}
