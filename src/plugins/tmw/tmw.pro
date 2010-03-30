TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH = ../..
TARGET = $$qtLibraryTarget(tmw)
DESTDIR = ../../../lib/tiled/plugins

DEFINES += TMW_LIBRARY

SOURCES += tmwplugin.cpp

HEADERS += tmwplugin.h\
        tmw_global.h
