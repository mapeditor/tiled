#pragma once

#include "mapformat.h"
#include "templategroup.h"

namespace Tiled {

class TILEDSHARED_EXPORT TemplateFormat : public FileFormat
{
    Q_OBJECT

public:
    explicit TemplateFormat(QObject *parent = nullptr)
        : FileFormat(parent)
    {}

    virtual TemplateGroup *read(const QString &fileName) = 0;
    virtual bool write(const QList<MapObject *> &mapObjects, const QString &fileName) = 0;
};

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::TemplateFormat, "org.mapeditor.TemplateFormat")
