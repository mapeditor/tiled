!contains(DISABLE_PYTHON_PLUGIN, yes) {
    unix {
        system(pkg-config python-2.7) {
            HAVE_PYTHON = yes
            CONFIG += link_pkgconfig
            PKGCONFIG += python-2.7
        } else:system(python-config --prefix) {
            # currently here for reference, only 2.7 is tested
            HAVE_PYTHON = yes
            QMAKE_CXXFLAGS = `python-config --cflags`
            QMAKE_LFLAGS = `python-config --libs`
        }
    }

    win32 {
        exists(C:/Python27/include/Python.h) {
            HAVE_PYTHON = yes
            QMAKE_CXXFLAGS += -IC:/Python27/include/
            CONFIG(debug, debug|release) {
                QMAKE_LIBS += -LC:/Python27/libs -lpython27_d
            } else {
                QMAKE_LIBS += -LC:/Python27/libs -lpython27
            }
        }
    }
}
