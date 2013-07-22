include(../plugin.pri)
include(find_python.pri)

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

DEFINES += PYTHON_LIBRARY

SOURCES += pythonplugin.cpp pythonbind.cpp
HEADERS += pythonplugin.h
