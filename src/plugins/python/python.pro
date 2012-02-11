include(../plugin.pri)

DEFINES += PYTHON_LIBRARY

SOURCES += pythonplugin.cpp pythonbind.cpp

HEADERS += pythonplugin.h

unix {
  CONFIG += link_pkgconfig
  PKGCONFIG += python-2.7
#	QMAKE_CXXFLAGS = `python-config --cflags`
#	QMAKE_LFLAGS = `python-config --libs`
}
*-g++* {
#  QMAKE_CXXFLAGS += -g -fPIC
#  QMAKE_LFLAGS += -g -fPIC
}

