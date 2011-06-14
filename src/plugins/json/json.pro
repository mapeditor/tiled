include(../plugin.pri)

DEFINES += JSON_LIBRARY

include(qjsonparser/qjsonparser.pri)

SOURCES += jsonplugin.cpp \
    maptovariant.cpp \
    varianttomap.cpp
HEADERS += jsonplugin.h\
        json_global.h
