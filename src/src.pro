TEMPLATE = app
TARGET = tiled
DESTDIR = ../bin
QT += core gui xml

SOURCES += main.cpp \
    mainwindow.cpp \
    xmlmapreader.cpp

HEADERS += mainwindow.h \
    mapreaderinterface.h \
    mapwriterinterface.h \
    xmlmapreader.h

FORMS += mainwindow.ui
