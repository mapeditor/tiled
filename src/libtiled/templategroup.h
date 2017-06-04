#pragma once

#include "tiled_global.h"

#include "objecttemplate.h"

namespace Tiled {

class MapObject;
class TemplateGroupFormat;

class TILEDSHARED_EXPORT TemplateGroup
{
public:
    TemplateGroup();
    ~TemplateGroup();
    void addTemplate(ObjectTemplate *objectTemplate);
    bool addTileset(const SharedTileset &tileset);
    const QList<ObjectTemplate*> &templates() const { return mTemplates; }

    void setFormat(TemplateGroupFormat *format);
    TemplateGroupFormat *format() const;

private:
    QList<ObjectTemplate*> mTemplates;
    QVector<SharedTileset> mTilesets;
    QPointer<TemplateGroupFormat> mFormat;
};

} // namespace Tiled
