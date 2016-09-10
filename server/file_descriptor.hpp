//
// file_descriptor.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FILE_DESC
#define FILE_DESC
#include <string>

namespace http {
namespace server {

struct file_descriptor {
    int value;
    std::string path;
    file_descriptor() = default;
    file_descriptor(const std::string &path, int mode);
    ~file_descriptor();
    file_descriptor(const file_descriptor &) = delete;
    file_descriptor &operator=(const file_descriptor &) = delete;

    inline bool good() const { return value != -1; }
};
}
}

#endif // FILE_DESC
