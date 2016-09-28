TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L/usr/local/lib -L/usr/local/opt/openssl/lib -lboost_system -lboost_filesystem -lboost_iostreams -lpthread -lssl -lcrypto
QMAKE_CXXFLAGS += -std=c++14 -Wall -I/usr/local/include -I/usr/local/opt/openssl/include
QMAKE_CXXFLAGS_DEBUG += -O0 -g -fno-optimize-sibling-calls -fno-omit-frame-pointer
QMAKE_CXXFLAGS_RELEASE -= -O2 -O1
QMAKE_CXXFLAGS_RELEASE += -s -O3 -flto
QMAKE_LFLAGS_DEBUG += -std=c++14
QMAKE_LFLAGS_RELEASE -= -O1
QMAKE_LFLAGS_RELEASE += -O3 -flto -std=c++14

SOURCES += main.cpp \
    directory_listing.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../server/release/ -lserver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../server/debug/ -lserver
else:unix: LIBS += -L$$OUT_PWD/../server/ -lserver

INCLUDEPATH += $$PWD/../server
DEPENDPATH += $$PWD/../server

HEADERS += \
    directory_listing.hpp
