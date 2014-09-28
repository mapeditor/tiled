#ifndef TILEDQUICKPLUGIN_PLUGIN_H
#define TILEDQUICKPLUGIN_PLUGIN_H

#include <QQmlExtensionPlugin>

class TiledQuickPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);
};

#endif // TILEDQUICKPLUGIN_PLUGIN_H
