isEmpty(TARGET) {
    error("plugin.pri: You must provide a TARGET")
}

TEMPLATE = lib
CONFIG += plugin
contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols
win32 {
    DESTDIR = $$OUT_PWD/../../../plugins/tiled
} else:macx {
    DESTDIR = $$OUT_PWD/../../../bin/Tiled.app/Contents/PlugIns
} else {
    DESTDIR = $$OUT_PWD/../../../lib/tiled/plugins
}

include(../../tiled.pri)
target.path = $${LIBDIR}/tiled/plugins
INSTALLS += target

include(../libtiled/libtiled.pri)
macx {
    QMAKE_LIBDIR += $$OUT_PWD/../../../bin/Tiled.app/Contents/Frameworks
} else:win32 {
    LIBS += -L$$OUT_PWD/../../../lib
} else {
    QMAKE_LIBDIR += $$OUT_PWD/../../../lib
}

# Set rpath so that the plugin will resolve libtiled correctly
!win32:!macx:contains(RPATH, yes) {
    QMAKE_RPATHDIR += \$\$ORIGIN/../..

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}
