//
// char_memory_mapping_cache.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAR_MEMORY_MAPPING_CACHE_H
#define CHAR_MEMORY_MAPPING_CACHE_H

#include "memory_mapping.hpp"
#include "file_descriptor_cache.hpp"
#include <fcntl.h>

namespace http {
namespace server {
struct char_memory_mapping_cache {
    static std::shared_ptr<char_memory_mapping> get(const std::string &path, int mode) {
        using namespace std;
        static unordered_map<string, weak_ptr<char_memory_mapping>> cache;
        static mutex m;

        lock_guard<mutex> hold(m);
        auto sp = cache[path].lock();
        if (!sp) {
            try {
                cache[path] = sp = make_shared<char_memory_mapping>(file_descriptor_cache::get(path, mode),
                                                                    boost::filesystem::file_size(path));
                if (!sp->good()) {
                    throw std::system_error(std::error_code(EBADF, std::system_category()), "Invalid file descriptor");
                }
            } catch (const std::system_error &) {
                throw;
            }
        }
        return sp;
    }
};
}
}

#endif /* CHAR_MEMORY_MAPPING_CACHE_H */
