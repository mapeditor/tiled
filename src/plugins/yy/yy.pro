include(../plugin.pri)

DEFINES += YY_LIBRARY

SOURCES += jsonwriter.cpp \
    yyplugin.cpp
HEADERS += jsonwriter.h \
    yyplugin.h \
    yy_global.h
