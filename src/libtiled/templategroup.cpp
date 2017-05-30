#include "templategroup.h"

using namespace Tiled;

TemplateGroup::TemplateGroup()
{
}

TemplateGroup::~TemplateGroup()
{
    qDeleteAll(mObjects);
}

void TemplateGroup::addObject(MapObject *object)
{
    mObjects.append(object);
}
