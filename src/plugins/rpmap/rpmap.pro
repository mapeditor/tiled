include(../plugin.pri)

DEFINES += RPMAP_LIBRARY

HEADERS += \
    rpmap_global.h \
    rpmapplugin.h

SOURCES += \
    rpmapplugin.cpp

OTHER_FILES = plugin.json

# we require KArchive https://invent.kde.org/frameworks/karchive
#QT += KArchive

unix:!macx: LIBS += -lKF5Archive

INCLUDEPATH += /usr/include/KF5/KArchive
DEPENDPATH += /usr/include/KF5/KArchive
#/usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/ ??
