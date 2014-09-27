#include "tiledquickplugin_plugin.h"
#include "tiledquickplugin.h"

#include <qqml.h>

void TiledquickpluginPlugin::registerTypes(const char *uri)
{
    // @uri org.mapeditor.Tiled
    qmlRegisterType<TiledQuickPlugin>(uri, 1, 0, "TiledQuickPlugin");
}


