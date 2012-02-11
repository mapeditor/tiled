include(../plugin.pri)

unix {
  system(pkg-config python-2.7) {
    HAVE_PYTHON = yes
    CONFIG += link_pkgconfig
    PKGCONFIG += python-2.7
  } else:system(python-config) {
    QMAKE_CXXFLAGS = `python-config --cflags`
    QMAKE_LFLAGS = `python-config --libs`
  }
}

contains(HAVE_PYTHON, yes) {
  DEFINES += PYTHON_LIBRARY
  SOURCES += pythonplugin.cpp pythonbind.cpp
  HEADERS += pythonplugin.h
} else {
 !build_pass:system(echo "No Python support")
}

