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
    const QList<ObjectTemplate*> &templates() const { return mTemplates; }

    void setFormat(TemplateGroupFormat *format);
    TemplateGroupFormat *format() const;

private:
    QList<ObjectTemplate*> mTemplates;
    QPointer<TemplateGroupFormat> mFormat;
};

} // namespace Tiled
