include(../../tiled.pri)
include(../libtiled/libtiled.pri)

TEMPLATE = lib
TARGET = tiledquickplugin
QT += qml quick
CONFIG += qt plugin

macx {
    DESTDIR = "../../bin/Tiled Quick.app/Contents/qml/org/mapeditor/Tiled"
} else {
    DESTDIR = ../../qml/org/mapeditor/Tiled
}

macx {
    QMAKE_LIBDIR += $$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
} else:win32 {
    LIBS += -L$$OUT_PWD/../../lib
} else {
    QMAKE_LIBDIR = $$OUT_PWD/../../lib $$QMAKE_LIBDIR
}

# Make sure the Tiled executable can find libtiled
!win32:!macx:contains(RPATH, yes) {
    QMAKE_RPATHDIR += \$\$ORIGIN/../../../../lib

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}

TARGET = $$qtLibraryTarget($$TARGET)
uri = org.mapeditor.Tiled

# Input
SOURCES += \
    tiledquickplugin.cpp \
    mapitem.cpp \
    maploader.cpp \
    tilelayeritem.cpp \
    tilesnode.cpp

HEADERS += \
    tiledquickplugin.h \
    mapitem.h \
    maploader.h \
    tilelayeritem.h \
    tilesnode.h

OTHER_FILES = qmldir

# Copy qmldir to "qml" directory, making sure to respect shadow builds.
copy_qmldir.target = $$OUT_PWD/$$DESTDIR/qmldir
copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
QMAKE_EXTRA_TARGETS += copy_qmldir
PRE_TARGETDEPS += $$copy_qmldir.target

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}
