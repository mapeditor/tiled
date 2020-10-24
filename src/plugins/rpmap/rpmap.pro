include(../plugin.pri)
#include(/usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/qt_KArchive.pri)

DEFINES += RPMAP_LIBRARY

HEADERS += \
    rpmap_global.h \
    rpmapplugin.h

SOURCES += \
    rpmapplugin.cpp

OTHER_FILES = plugin.json

# we require KArchive https://invent.kde.org/frameworks/karchive
#QT += KArchive

unix:!macx: LIBS += -L$$PWD/../../../../../../../usr/lib/x86_64-linux-gnu/ -lKF5Archive

INCLUDEPATH += $$PWD/../../../../../../../usr/include/KF5/KArchive
DEPENDPATH += $$PWD/../../../../../../../usr/include/KF5/KArchive
