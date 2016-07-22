//
// file_desc_cache.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FILE_DESC_CACHE
#define FILE_DESC_CACHE
#include "file_desc.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace http {
namespace server {

struct file_desc_cache {
    static std::shared_ptr<file_desc> get(const std::string &path, int mode) {
        using namespace std;
        static unordered_map<string, weak_ptr<file_desc>> cache;
        static mutex m;

        lock_guard<mutex> hold(m);
        auto sp = cache[path].lock();
        if (!sp)
            cache[path] = sp = make_shared<file_desc>(path, mode);
        return sp;
    }
};
}
}

#endif // FILE_DESC_CACHE
