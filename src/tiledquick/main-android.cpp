#include "tiledquickplugin.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationDomain("mapeditor.org");
    QCoreApplication::setApplicationName("TiledQuick");

    TiledQuick::TiledQuickPlugin plugin;
    plugin.registerTypes("org.mapeditor.Tiled");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main-android.qml")));

    return app.exec();
}
