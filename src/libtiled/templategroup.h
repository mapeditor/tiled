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
    TemplateGroup(QString name);
    ~TemplateGroup();

    const QList<ObjectTemplate*> &templates() const;
    void addTemplate(ObjectTemplate *objectTemplate);

    const QVector<SharedTileset> &tilesets() const;
    bool addTileset(const SharedTileset &tileset);

    const QString &name() const;
    void setName(const QString &name);

private:
    QList<ObjectTemplate*> mTemplates;
    QVector<SharedTileset> mTilesets;
    QPointer<TemplateGroupFormat> mFormat;
    QString mName;
};

inline const QList<ObjectTemplate*> &TemplateGroup::templates() const
{ return mTemplates; }

inline const QVector<SharedTileset> &TemplateGroup::tilesets() const
{ return mTilesets; }

inline const QString &TemplateGroup::name() const
{ return mName; }

inline void TemplateGroup::setName(const QString &name)
{ mName = name; }

} // namespace Tiled
