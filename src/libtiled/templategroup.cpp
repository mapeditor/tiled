#include "templategroup.h"

#include "templategroupformat.h"

using namespace Tiled;

TemplateGroup::TemplateGroup()
{
}

TemplateGroup::TemplateGroup(QString name):
    mName(name)
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
