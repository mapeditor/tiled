#pragma once

#include "tiled_global.h"

#include "mapobject.h"

namespace Tiled {

class MapObject;
class TemplateFormat;

class TILEDSHARED_EXPORT TemplateGroup
{
public:
    TemplateGroup();
    ~TemplateGroup();
    void addObject(MapObject *object);
    const QList<MapObject*> &objects() const { return mObjects; }

    void setFormat(TemplateFormat *format);
    TemplateFormat *format() const;

private:
    QList<MapObject*> mObjects;
    QPointer<TemplateFormat> mFormat;
};

} // namespace Tiled
