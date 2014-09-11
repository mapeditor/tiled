#include "createellipseobjecttool.h"
#include "preferences.h"
#include "Utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateEllipseObjectTool::CreateEllipseObjectTool(QObject* parent)
    : CreateScalableObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-ellipse.png")));
    Utils::setThemeIcon(this, "insert-ellipse");
    languageChanged();
}

void CreateEllipseObjectTool::languageChanged()
{
    setName(tr("Insert Ellipse"));
    setShortcut(QKeySequence(tr("C")));
}

MapObject* CreateEllipseObjectTool::createNewMapObject(const QPointF &pos)
{
   MapObject* newMapObject = new MapObject();
    newMapObject->setShape(MapObject::Ellipse);
    return newMapObject;
}
