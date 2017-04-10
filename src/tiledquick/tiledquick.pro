TEMPLATE = app

QT += qml quick widgets

win32 {
    DESTDIR = ../..
} else {
    DESTDIR = ../../bin
}

macx {
    TARGET = "Tiled Quick"
}

SOURCES += main.cpp

OTHER_FILES = main.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
