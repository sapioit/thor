//
// file_descriptor_cache.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FILE_DESC_CACHE
#define FILE_DESC_CACHE
#include "file_descriptor.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <system_error>

namespace http {
namespace server {

struct file_descriptor_cache {
    static std::shared_ptr<file_descriptor> get(const std::string &path, int mode) {
        using namespace std;
        static unordered_map<string, weak_ptr<file_descriptor>> cache;
        static mutex m;

        lock_guard<mutex> hold(m);
        auto sp = cache[path].lock();
        if (!sp) {
            cache[path] = sp = make_shared<file_descriptor>(path, mode);
            if (!sp->good())
                throw std::system_error(std::error_code(EBADF, std::system_category()), "Could not open file");
        }
        return sp;
    }
};
}
}

#endif // FILE_DESC_CACHE
