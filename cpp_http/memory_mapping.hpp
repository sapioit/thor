//
// memory_mapping.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MEM_MAPPING_H
#define MEM_MAPPING_H

#include "file_descriptor.hpp"
#include <string>
#include <memory>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <boost/filesystem.hpp>

namespace http {
namespace server {

template <typename T> class memory_mapping {
    public:
    typedef T type;

    memory_mapping() : mem(nullptr), len(0) {}
    memory_mapping(std::shared_ptr<file_descriptor> fd, std::size_t len) : fd_(fd), mem(nullptr), len(len) {
        map_in_mem();
    }
    memory_mapping(const memory_mapping &) = delete;
    memory_mapping &operator=(const memory_mapping &) = delete;
    memory_mapping(memory_mapping &&other) { (*this) = std::move(other); }
    memory_mapping &operator=(memory_mapping &&other) {
        if (this != &other) {
            fd_ = other.fd_;
            other.fd_ = -1;

            mem = other.mem;
            other.mem = nullptr;

            len = other.len;
            other.len = 0;
        }
    }

    virtual ~memory_mapping() { unmap_close(); }

    void sync() const noexcept {
        if (fd_ != -1)
            fsync(fd_->value);
    }

    bool good() const noexcept { return !(mem == MAP_FAILED || mem == nullptr); }

    std::size_t length() const noexcept { return len; }

    std::size_t size() const noexcept { return len / sizeof(type); }

    inline type &operator[](std::size_t idx) noexcept { return *(static_cast<type *>(mem) + idx * sizeof(type)); }

    inline type &at(std::size_t idx) {
        if (idx < len * sizeof(type)) {
            return (*this)[idx];
        } else
            throw std::out_of_range{"Index out of bounds"};
    }

    private:
    std::shared_ptr<file_descriptor> fd_;
    void *mem;
    std::size_t len;

    void map_in_mem() {
        int fd = fd_->value;
        mem = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
        if (!good())
            throw std::system_error(std::error_code(errno, std::system_category()));
    }

    void unmap_close() {
        if (this->good())
            munmap(mem, len);
    }
};
typedef memory_mapping<char> char_memory_mapping;
}
}

#endif // MEM_MAPPING_H