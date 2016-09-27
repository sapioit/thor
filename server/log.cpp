//
// log.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "log.hpp"
#include <fstream>
#include <mutex>

std::ofstream &log::get_stream() {
    static std::ofstream f("thor.log.txt");
    return f;
}

void log::write(const std::string &text) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> hold(mutex);

    get_stream() << text << std::endl;
}
