# See the README file for instructions about setting the install prefix.
isEmpty(PREFIX):PREFIX = /usr/local
isEmpty(LIBDIR):LIBDIR = $${PREFIX}/lib
isEmpty(RPATH):RPATH = yes
isEmpty(INSTALL_HEADERS):INSTALL_HEADERS = no

macx {
    # Do a universal build when possible
    contains(QT_CONFIG, ppc):CONFIG += x86 ppc
}

CONFIG += depend_includepath


!isEmpty(USE_FHS_PLUGIN_PATH) {
    DEFINES += TILED_PLUGIN_DIR=\\\"$${LIBDIR}/tiled/plugins/\\\"
}
