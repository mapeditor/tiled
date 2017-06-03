#include "templategroup.h"

#include "templateformat.h"

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

void TemplateGroup::setFormat(TemplateFormat *format)
{
    mFormat = format;
}

TemplateFormat *TemplateGroup::format() const
{
    return mFormat;
}
