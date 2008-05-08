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
    tileset.cpp \
    resizehelper.cpp \
    resizedialog.cpp
HEADERS += mainwindow.h \
    mapreaderinterface.h \
    mapwriterinterface.h \
    xmlmapreader.h \
    map.h \
    tileset.h \
    resizedialog.h \
    resizehelper.h
FORMS += mainwindow.ui \
    resizedialog.ui
