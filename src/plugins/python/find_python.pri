!contains(DISABLE_PYTHON_PLUGIN, yes) {
    unix {
        packagesExist(python3) {
            HAVE_PYTHON = yes
            CONFIG += link_pkgconfig
            PKGCONFIG += python3
        }
    }

    win32 {
        exists(C:/Python27/include/Python.h) {
            HAVE_PYTHON = yes
            QMAKE_CXXFLAGS += -IC:/Python27/include/
            QMAKE_LIBS += -LC:/Python27/libs -lpython27
        }
    }
}
