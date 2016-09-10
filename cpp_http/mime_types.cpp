//
// mime_types.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "mime_types.hpp"
std::string http::server::mime_types::get_mime_type(const std::string &path) {
    auto ext = get_extension(path);
    if (ext != "") {
        auto it = mappings.find(ext);
        if (it != mappings.end())
            return it->second;
    }
    return shell_get_mimetype(path);
}

std::string http::server::mime_types::shell_get_mimetype(const std::string &p) noexcept {
    std::string cmd = "file --mime-type " + p;

    auto output = exec(cmd);
    while (output.size() && std::isspace(output.back()))
        output = output.substr(0, output.size() - 1);
    auto c = output.find_first_of(':');
    if (c != std::string::npos && output.size() > ++c)
        return output.substr(c + 1);
    return "text/plain";
}

std::string http::server::mime_types::exec(const std::string &cmd) {
    std::unique_ptr<FILE, std::function<void(FILE *)>> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        return "";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

std::string http::server::mime_types::get_extension(const std::string &path) {
    // Determine the file extension.
    std::size_t last_slash_pos = path.find_last_of("/");
    std::size_t last_dot_pos = path.find_last_of(".");
    std::string extension;

    if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
        extension = path.substr(last_dot_pos + 1);
    }
    return extension;
}
