include(../plugin.pri)

DEFINES += LUA_LIBRARY

SOURCES += luaplugin.cpp \
    luatablewriter.cpp
HEADERS += luaplugin.h\
    lua_global.h \
    luatablewriter.h
