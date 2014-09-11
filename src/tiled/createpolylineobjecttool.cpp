#include "createpolylineobjecttool.h"
#include "preferences.h"
#include "Utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreatePolylineObjectTool::CreatePolylineObjectTool(QObject* parent)
    : CreateMultipointObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-polyline.png")));
    languageChanged();
}

void CreatePolylineObjectTool::languageChanged()
{
    setName(tr("Insert Polyline"));
    setShortcut(QKeySequence(tr("L")));
}

MapObject* CreatePolylineObjectTool::createNewMapObject()
{
    MapObject* newMapObject = new MapObject();
    newMapObject->setShape(MapObject::Polyline);
    return newMapObject;
}

void CreatePolylineObjectTool::finishNewMapObject(){
    if(mNewMapObjectItem->mapObject()->polygon().size() >= 2){
        CreateObjectTool::finishNewMapObject();
    }
    else{
        CreateObjectTool::cancelNewMapObject();
    }
}
