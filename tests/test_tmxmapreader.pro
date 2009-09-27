CONFIG += qtestlib
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += . \
    ../src

# Input
SOURCES += test_tmxmapreader.cpp

# Tiled sources to include in the test binary
SOURCES += ../src/map.cpp \
    ../src/mapobject.cpp \
    ../src/layer.cpp \
    ../src/tilelayer.cpp \
    ../src/objectgroup.cpp \
    ../src/tilesetmanager.cpp \
    ../src/tileset.cpp \
    ../src/compression.cpp \
    ../src/tmxmapreader.cpp
