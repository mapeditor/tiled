#include "rotatemapobject.h"

#include "mapdocument.h"
#include "mapobject.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {


RotateMapObject::RotateMapObject(MapDocument *mapDocument,
                MapObject *mapObject,
                qreal oldAngle)
    : mMapDocument(mapDocument),
      mMapObject(mapObject),
      mOldAngle(oldAngle),
      mNewAngle(mapObject->angle())
{
    setText(QCoreApplication::translate("Undo Commands", "Rotate Object"));
}

void RotateMapObject::undo()
{
    mMapObject->setAngle(mOldAngle);
    mMapDocument->emitObjectChanged(mMapObject);
}

void RotateMapObject::redo() {
    mMapObject->setAngle(mNewAngle);
    mMapDocument->emitObjectChanged(mMapObject);
}


} // namespace Internal
} // namespace Tiled
