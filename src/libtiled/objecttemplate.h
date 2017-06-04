#pragma once

#include "mapobject.h"

using namespace Tiled;

class TILEDSHARED_EXPORT ObjectTemplate
{
public:
    ObjectTemplate();
    ObjectTemplate(int id, QString name);

    const MapObject *object() const;
    void setObject(MapObject *object);

    int id() const;
    void setId(int id);

    const QString &name() const;
    void setName(const QString &name);

private:
    MapObject *mObject;
    int mId;
    QString mName;
};

inline const MapObject *ObjectTemplate::object() const
{ return mObject; }

inline void ObjectTemplate::setObject(MapObject *object)
{
    mObject = object->clone();
    mObject->setId(0);
}

inline int ObjectTemplate::id() const
{ return mId; }

inline void ObjectTemplate::setId(int id)
{ mId = id; }

inline const QString &ObjectTemplate::name() const
{ return mName; }

inline void ObjectTemplate::setName(const QString &name)
{ mName = name; }
