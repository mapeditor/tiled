include(../plugin.pri)

DEFINES += JSON_LIBRARY

SOURCES += jsonplugin.cpp \
    qjsonparser/json.cpp

HEADERS += jsonplugin.h \
    json_global.h \
    qjsonparser/json.h
