include(../plugin.pri)
include(find_python.pri)

QT += widgets

DEFINES += PYTHON_LIBRARY

SOURCES += pythonplugin.cpp pythonbind.cpp
HEADERS += pythonplugin.h
