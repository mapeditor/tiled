#pragma once

#include "tiled_global.h"

#include "mapobject.h"

namespace Tiled {

class MapObject;

class TILEDSHARED_EXPORT TemplateGroup
{
public:
    TemplateGroup();
    ~TemplateGroup();
    void addObject(MapObject *object);
    const QList<MapObject*> &objects() const { return mObjects; }
private:
    QList<MapObject*> mObjects;
};

} // namespace Tiled
