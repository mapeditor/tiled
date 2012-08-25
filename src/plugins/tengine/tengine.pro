include(../plugin.pri)

DEFINES += TENGINE_LIBRARY

SOURCES += tengineplugin.cpp
HEADERS += tengineplugin.h\
        tengine_global.h

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}
