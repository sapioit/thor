//
// char_memory_mapping_cache.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "char_memory_mapping_cache.hpp"
#include <fcntl.h>
#include <mutex>
#include <unordered_map>

std::shared_ptr<http::server::char_memory_mapping> http::server::char_memory_mapping_cache::get(const std::string &path,
                                                                                                int mode) {
    using namespace std;
    static unordered_map<string, weak_ptr<char_memory_mapping>> cache;
    static mutex m;

    lock_guard<mutex> hold(m);
    auto sp = cache[path].lock();
    if (!sp) {
        try {
            cache[path] = sp = make_shared<char_memory_mapping>(file_descriptor_cache::get(path, mode),
                                                                boost::filesystem::file_size(path));
        } catch (const std::system_error &) {
            throw;
        }
    }

    return sp;
}
