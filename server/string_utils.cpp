//
// string_utils.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "string_utils.hpp"

std::string http::server::uppercase(const std::string &str) { return uppercase(str.cbegin(), str.cend()); }
