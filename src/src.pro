TEMPLATE = app
TARGET = tiled
DESTDIR = ../bin
DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
QT += core \
    gui \
    xml
CONFIG += debug
SOURCES += decompress.cpp \
    main.cpp \
    mainwindow.cpp \
    xmlmapreader.cpp \
    map.cpp \
    mapobject.cpp \
    mapobjectitem.cpp \
    mapscene.cpp \
    layer.cpp \
    layerdock.cpp \
    layertablemodel.cpp \
    objectgroup.cpp \
    tileset.cpp \
    tilelayeritem.cpp \
    resizehelper.cpp \
    resizedialog.cpp \
    propertiesmodel.cpp \
    propertiesdialog.cpp
HEADERS += decompress.h \
    mainwindow.h \
    mapreaderinterface.h \
    mapwriterinterface.h \
    xmlmapreader.h \
    map.h \
    mapobject.h \
    mapobjectitem.h \
    mapscene.h \
    layer.h \
    layerdock.h \
    layertablemodel.h \
    objectgroup.h \
    tileset.h \
    tilelayeritem.h \
    resizedialog.h \
    resizehelper.h \
    propertiesmodel.h \
    propertiesdialog.h
FORMS += mainwindow.ui \
    resizedialog.ui \
    propertiesdialog.ui
RESOURCES += tiled.qrc
TRANSLATIONS = translations/tiled_nl.ts
