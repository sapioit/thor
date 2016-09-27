//
// log.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef LOG_HPP
#define LOG_HPP
#include <fstream>
#include <string>

class log {
    static std::ofstream &get_stream();

    public:
    static void write(const std::string &text);
};

#endif // LOG_HPP
