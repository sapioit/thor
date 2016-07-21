TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lboost_system -lboost_filesystem -lpthread -flto
QMAKE_CXXFLAGS += -std=c++14 -Wall
QMAKE_CXXFLAGS_DEBUG += -O0 -g
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -flto


HEADERS += \
    connection.hpp \
    header.hpp \
    mime_types.hpp \
    reply.hpp \
    request_handler.hpp \
    request_parser.hpp \
    request.hpp \
    server.hpp \
    user_handler.hpp \
    sendfile_op.hpp \
    file_desc.h \
    file_desc_cache.h

SOURCES += \
    connection.cpp \
    main.cpp \
    mime_types.cpp \
    reply.cpp \
    request_handler.cpp \
    request_parser.cpp \
    server.cpp \
    user_handler.cpp
