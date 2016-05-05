TEMPLATE = lib
TARGET = logsdk
CONFIG += c++11


CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += "../../bin/debug"
    DESTDIR = "../../bin/debug"
} else {
    QMAKE_LIBDIR += "../../bin/release"
    DESTDIR = "../../bin/release"
}

# Input
HEADERS += logger.h
HEADERS += xlogwriter.h
HEADERS += util/xmutex.h
HEADERS += util/utility.h

SOURCES += logger.cc
SOURCES += xlogwriter.cc
SOURCES += util/xmutex.cc
SOURCES += util/utility.cc

