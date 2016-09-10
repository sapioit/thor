//
// string_utils.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP
#include <string>

namespace http {
namespace server {
template <typename InputIterator> inline std::string uppercase(InputIterator begin, InputIterator end) {
    std::string out(end - begin, '0');
    auto output_it = out.begin();
    while (begin != end) {
        *output_it++ = std::toupper(*begin++);
    }
    return out;
}

std::string uppercase(const std::string &str);
}
}

#endif // STRING_UTILS_HPP
