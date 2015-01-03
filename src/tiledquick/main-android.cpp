#include "tiledquickplugin.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    TiledQuick::TiledQuickPlugin plugin;
    plugin.registerTypes("org.mapeditor.Tiled");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
