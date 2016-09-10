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

namespace http {
namespace server {

struct file_descriptor_cache {
    static std::shared_ptr<file_descriptor> get(const std::string &path, int mode);
};
}
}

#endif // FILE_DESC_CACHE
