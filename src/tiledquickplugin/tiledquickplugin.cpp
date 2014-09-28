#include "tiledquickplugin.h"

#include "objectgroup.h"

#include <qqml.h>

void TiledQuickPlugin::registerTypes(const char *uri)
{
    // @uri org.mapeditor.Tiled
    qmlRegisterType<Tiled::ObjectGroup>(uri, 1, 0, "ObjectGroup");
}
