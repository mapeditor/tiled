#pragma once

#include "mapobject.h"

using namespace Tiled;

class TILEDSHARED_EXPORT ObjectTemplate
{
public:
    ObjectTemplate();
    ObjectTemplate(MapObject object);
    const MapObject *object() const { return mObject; }
    void setObject(MapObject *object);
private:
    MapObject *mObject;
};

inline void ObjectTemplate::setObject(MapObject *object)
{ mObject = object; }
