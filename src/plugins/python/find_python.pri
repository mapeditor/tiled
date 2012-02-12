unix {
    system(pkg-config python-2.7) {
        HAVE_PYTHON = yes
        CONFIG += link_pkgconfig
        PKGCONFIG += python-2.7
    } else:system(python-config) {
        # currently here for reference, only 2.7 is tested
        HAVE_PYTHON = yes
        QMAKE_CXXFLAGS = `python-config --cflags`
        QMAKE_LFLAGS = `python-config --libs`
    }
}
