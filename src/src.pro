TEMPLATE = app
TARGET = tiled
DESTDIR = ../bin
QT += core \
    gui \
    xml
SOURCES += main.cpp \
    mainwindow.cpp \
    xmlmapreader.cpp \
    map.cpp \
    tileset.cpp
HEADERS += mainwindow.h \
    mapreaderinterface.h \
    mapwriterinterface.h \
    xmlmapreader.h \
    map.h \
    tileset.h \
    mapviewinterface.h
FORMS += mainwindow.ui
