TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lboost_system -lpthread

HEADERS += \
    connection.hpp \
    header.hpp \
    mime_types.hpp \
    reply.hpp \
    request_handler.hpp \
    request_parser.hpp \
    request.hpp \
    server.hpp

SOURCES += \
    connection.cpp \
    main.cpp \
    mime_types.cpp \
    reply.cpp \
    request_handler.cpp \
    request_parser.cpp \
    server.cpp
