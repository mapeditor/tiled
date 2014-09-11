#include "createpolygonobjecttool.h"
#include "preferences.h"
#include "Utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreatePolygonObjectTool::CreatePolygonObjectTool(QObject* parent)
    : CreateMultipointObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-polygon.png")));
    languageChanged();
}

void CreatePolygonObjectTool::languageChanged()
{
    setName(tr("Insert Polygon"));
    setShortcut(QKeySequence(tr("P")));
}

MapObject* CreatePolygonObjectTool::createNewMapObject(const QPointF &pos)
{
    MapObject* newMapObject = new MapObject();
    newMapObject->setShape(MapObject::Polygon);
    return newMapObject;
}

void CreatePolygonObjectTool::finishNewMapObject(){
    if(mNewMapObjectItem->mapObject()->polygon().size() >= 3){
        CreateObjectTool::finishNewMapObject();
    }
    else{
        CreateObjectTool::cancelNewMapObject();
    }
}
