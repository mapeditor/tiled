#include "CreateRectangleobjecttool.h"
#include "preferences.h"
#include "Utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateRectangleObjectTool::CreateRectangleObjectTool(QObject* parent)
    : CreateScalableObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-rectangle.png")));
    Utils::setThemeIcon(this, "insert-rectangle");
}

void CreateRectangleObjectTool::languageChanged()
{
    setName(tr("Insert Rectangle"));
    setShortcut(QKeySequence(tr("R")));
}

MapObject* CreateRectangleObjectTool::createNewMapObject(const QPointF &pos)
{
   MapObject* newMapObject = new MapObject();
    newMapObject->setShape(MapObject::Rectangle);
    return newMapObject;
}
