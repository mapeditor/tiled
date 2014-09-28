#include "tiledquickplugin.h"

#include "map.h"

#include <qqml.h>

void TiledQuickPlugin::registerTypes(const char *uri)
{
    // @uri org.mapeditor.Tiled
    qmlRegisterType<QObject>(uri, 1, 0, "TiledQuickPlugin");
}
