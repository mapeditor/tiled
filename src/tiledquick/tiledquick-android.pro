TEMPLATE = app

QT += qml quick

include(../tiledquickplugin/tiledquickplugin-static.pri)

SOURCES += \
    main-android.cpp

RESOURCES += \
    tiledquick-android.qrc

# Default rules for deployment.
include(deployment.pri)
