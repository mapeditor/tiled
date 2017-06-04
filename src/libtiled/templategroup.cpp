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

void TemplateGroup::setFormat(TemplateGroupFormat *format)
{
    mFormat = format;
}

TemplateGroupFormat *TemplateGroup::format() const
{
    return mFormat;
}
