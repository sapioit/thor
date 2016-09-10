TEMPLATE = app
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L/usr/local/lib -L/usr/local/opt/openssl/lib -lboost_system -lboost_filesystem -lpthread -lssl -lcrypto
QMAKE_CXXFLAGS += -std=c++14 -Wall -I/usr/local/include -I/usr/local/opt/openssl/include
QMAKE_CXXFLAGS_DEBUG += -O0 -g -fno-optimize-sibling-calls -fno-omit-frame-pointer
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -s -O3


HEADERS += \
    cpp_http/connection.hpp \
    cpp_http/header.hpp \
    cpp_http/mime_types.hpp \
    cpp_http/reply.hpp \
    cpp_http/request_parser.hpp \
    cpp_http/request.hpp \
    cpp_http/server.hpp \
    cpp_http/user_handler.hpp \
    cpp_http/sendfile_op.hpp \
    cpp_http/ssl_connection.h \
    cpp_http/request_handler.hpp \
    cpp_http/file_desc.hpp \
    cpp_http/file_desc_cache.hpp \
    cpp_http/file_descriptor_cache.hpp \
    cpp_http/file_descriptor.hpp \
    cpp_http/ssl_connection.hpp \
    cpp_http/char_memory_mapping_cache.hpp \
    cpp_http/memory_mapping.hpp \
    cpp_http/string_utils.hpp \
    cpp_http/directory_listing.hpp

SOURCES += \
    cpp_http/main.cpp \
    cpp_http/char_memory_mapping_cache.cpp \
    cpp_http/connection.cpp \
    cpp_http/directory_listing.cpp \
    cpp_http/file_descriptor_cache.cpp \
    cpp_http/file_descriptor.cpp \
    cpp_http/header.cpp \
    cpp_http/io_service_pool.cpp \
    cpp_http/mime_types.cpp \
    cpp_http/reply.cpp \
    cpp_http/request_handler.cpp \
    cpp_http/request_parser.cpp \
    cpp_http/request.cpp \
    cpp_http/sendfile_op.cpp \
    cpp_http/server.cpp \
    cpp_http/ssl_connection.cpp \
    cpp_http/string_utils.cpp \
    cpp_http/user_handler.cpp \
    cpp_http/memory_mapping.cpp
