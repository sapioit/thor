#-------------------------------------------------
#
# Project created by QtCreator 2016-09-10T13:53:39
#
#-------------------------------------------------

QT       -= core gui

TARGET = server
TEMPLATE = lib

DEFINES += SERVER_LIBRARY

QMAKE_CXXFLAGS += -std=c++14 -Wall -I/usr/local/include -I/usr/local/opt/openssl/include
QMAKE_CXXFLAGS_DEBUG += -O0 -g -fno-optimize-sibling-calls -fno-omit-frame-pointer
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -s -O3 -flto
QMAKE_LFLAGS_RELEASE -= -O1
QMAKE_LFLAGS_RELEASE += -O3 -flto -std=c++14

SOURCES += server.cpp \
    char_memory_mapping_cache.cpp \
    connection.cpp \
    file_descriptor_cache.cpp \
    file_descriptor.cpp \
    header.cpp \
    io_service_pool.cpp \
    memory_mapping.cpp \
    mime_types.cpp \
    reply.cpp \
    request_handler.cpp \
    request_parser.cpp \
    request.cpp \
    sendfile_op.cpp \
    ssl_connection.cpp \
    string_utils.cpp \
    user_handler.cpp \
    log.cpp

HEADERS += \
    thor.hpp \
    char_memory_mapping_cache.hpp \
    connection.hpp \
    file_descriptor_cache.hpp \
    file_descriptor.hpp \
    header.hpp \
    io_service_pool.hpp \
    memory_mapping.hpp \
    mime_types.hpp \
    reply.hpp \
    request_handler.hpp \
    request_parser.hpp \
    request.hpp \
    sendfile_op.hpp \
    server.hpp \
    ssl_connection.hpp \
    string_utils.hpp \
    user_handler.hpp \
    log.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
