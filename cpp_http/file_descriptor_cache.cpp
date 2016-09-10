//
// file_descriptor_cache.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "file_descriptor_cache.hpp"
#include <mutex>
#include <system_error>
#include <unordered_map>

std::shared_ptr<http::server::file_descriptor> http::server::file_descriptor_cache::get(const std::string &path,
                                                                                        int mode) {
    using namespace std;
    static unordered_map<string, weak_ptr<file_descriptor>> cache;
    static mutex m;

    lock_guard<mutex> hold(m);
    auto sp = cache[path].lock();
    if (!sp) {
        try {
            cache[path] = sp = make_shared<file_descriptor>(path, mode);
        } catch (const std::system_error &) {
            throw;
        }
    }
    return sp;
}
