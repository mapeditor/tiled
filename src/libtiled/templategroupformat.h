#pragma once

#include "mapformat.h"
#include "templategroup.h"

namespace Tiled {

class TILEDSHARED_EXPORT TemplateGroupFormat : public FileFormat
{
    Q_OBJECT

public:
    explicit TemplateGroupFormat(QObject *parent = nullptr)
        : FileFormat(parent)
    {}

    virtual TemplateGroup *read(const QString &fileName) = 0;
    virtual bool write(const QList<MapObject *> &mapObjects, const QString &fileName) = 0;
};

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::TemplateGroupFormat, "org.mapeditor.TemplateGroupFormat")
