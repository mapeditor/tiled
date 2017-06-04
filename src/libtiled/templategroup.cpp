#include "templategroup.h"

#include "templategroupformat.h"

using namespace Tiled;

TemplateGroup::TemplateGroup()
{
}

TemplateGroup::~TemplateGroup()
{
    qDeleteAll(mTemplates);
}

void TemplateGroup::addTemplate(ObjectTemplate *objectTemplate)
{
    mTemplates.append(objectTemplate);
}

bool TemplateGroup::addTileset(const SharedTileset &tileset)
{
    if (mTilesets.contains(tileset))
        return false;

    mTilesets.append(tileset);
    return true;
}

void TemplateGroup::setFormat(TemplateGroupFormat *format)
{
    mFormat = format;
}

TemplateGroupFormat *TemplateGroup::format() const
{
    return mFormat;
}
