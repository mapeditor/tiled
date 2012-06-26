include(../plugin.pri)

DEFINES += TENGINE_LIBRARY

SOURCES += tengineplugin.cpp
HEADERS += tengineplugin.h\
        tengine_global.h

contains(QT_VERSION, ^5\\..*) {
	QT       += widgets
}

