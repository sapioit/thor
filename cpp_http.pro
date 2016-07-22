TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lboost_system -lboost_filesystem -lpthread -lssl -lcrypto -flto
QMAKE_CXXFLAGS += -std=c++14 -Wall
QMAKE_CXXFLAGS_DEBUG += -O0 -g
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -s -O3 -flto


HEADERS += \
    connection.hpp \
    header.hpp \
    mime_types.hpp \
    reply.hpp \
    request_parser.hpp \
    request.hpp \
    server.hpp \
    user_handler.hpp \
    sendfile_op.hpp \
    ssl_connection.h \
    request_handler.hpp \
    file_desc.hpp \
    file_desc_cache.hpp

SOURCES += \
    main.cpp
