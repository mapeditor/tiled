# Check the Qt version. If QT_VERSION is not set, it is probably Qt 3.
isEmpty(QT_VERSION) {
    error("QT_VERSION not defined. Tiled does not work with Qt 3.")
}

include(tiled.pri)

!minQtVersion(4, 7, 0) {
    message("Cannot build Tiled with Qt version $${QT_VERSION}")
    error("Use at least Qt 4.7.0.")
}

TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = src translations
