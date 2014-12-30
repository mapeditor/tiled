# See the README file for instructions about setting the install prefix.
isEmpty(PREFIX):PREFIX = /usr/local
isEmpty(LIBDIR):LIBDIR = $${PREFIX}/lib
isEmpty(RPATH):RPATH = yes
isEmpty(INSTALL_HEADERS):INSTALL_HEADERS = no

macx {
    # Do a universal build when possible
    contains(QT_CONFIG, ppc):CONFIG += x86 ppc
}

# This allows Tiled to use up to 3 GB on 32-bit systems and 4 GB on
# 64-bit systems, rather than being limited to just 2 GB.
win32-g++* {
    QMAKE_LFLAGS += -Wl,--large-address-aware
} else:win32 {
    QMAKE_LFLAGS += /LARGEADDRESSAWARE
}

CONFIG += depend_includepath

!isEmpty(USE_FHS_PLUGIN_PATH) {
    DEFINES += TILED_PLUGIN_DIR=\\\"$${LIBDIR}/tiled/plugins/\\\"
}
