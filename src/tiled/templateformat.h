#pragma once

#include "mapformat.h"
#include "mapobject.h"

namespace Tiled {

class TILEDSHARED_EXPORT TemplateFormat : public FileFormat
{
    Q_OBJECT

public:
    explicit TemplateFormat(QObject *parent = nullptr)
        : FileFormat(parent)
    {}

    virtual bool write(const MapObject *mapObject, const QString &fileName) = 0;
};

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::TemplateFormat, "org.mapeditor.TemplateFormat")
