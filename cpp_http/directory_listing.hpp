//
// directory_listing.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef DIRECTORY_LISTING_HPP
#define DIRECTORY_LISTING_HPP

#include "reply.hpp"
#include "request.hpp"
#include <boost/filesystem/path.hpp>
#include <string>

namespace http {
namespace server {

void list_directory(const request &req, reply &reply, const std::string &doc_root);
}
}

#endif // DIRECTORY_LISTING_HPP
