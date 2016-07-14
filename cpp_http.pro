TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    mime_types.cpp \
    reply.cpp \
    request_handler.cpp \
    request_parser.cpp \
    server.cpp \
    connection_manager.cpp \
    connection.cpp

HEADERS += \
    mime_types.hpp \
    reply.hpp \
    request_handler.hpp \
    request_parser.hpp \
    request.hpp \
    server.hpp \
    header.hpp \
    connection_manager.hpp \
    connection.hpp

LIBS += -lboost_system -lpthread
