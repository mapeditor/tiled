#include "templategroup.h"

#include "templategroupformat.h"

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

void TemplateGroup::setFormat(TemplateGroupFormat *format)
{
    mFormat = format;
}

TemplateGroupFormat *TemplateGroup::format() const
{
    return mFormat;
}
