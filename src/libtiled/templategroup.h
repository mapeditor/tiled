#pragma once

#include "tiled_global.h"

#include "mapobject.h"

namespace Tiled {

class MapObject;
class TemplateGroupFormat;

class TILEDSHARED_EXPORT TemplateGroup
{
public:
    TemplateGroup();
    ~TemplateGroup();
    void addObject(MapObject *object);
    const QList<MapObject*> &objects() const { return mObjects; }

    void setFormat(TemplateGroupFormat *format);
    TemplateGroupFormat *format() const;

private:
    QList<MapObject*> mObjects;
    QPointer<TemplateGroupFormat> mFormat;
};

} // namespace Tiled
