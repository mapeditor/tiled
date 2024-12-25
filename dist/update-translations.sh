# Updates the .ts files based on the strings found in the source code
# Meant to be run from repository root
lupdate -locations relative -no-obsolete \
    src/libtiled \
    src/plugins \
    src/qtpropertybrowser \
    src/qtsingleapplication \
    src/terraingenerator \
    src/tiled \
    src/tiledapp \
    src/tiledquick \
    src/tiledquickplugin \
    src/tmxrasterizer \
    src/tmxviewer \
    -ts translations/*.ts
