/*
 * editablemapobject.h
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "mapobject.h"

#include <QObject>

namespace Tiled {

class EditableMap;
class EditableObjectGroup;

class EditableMapObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ id)
//    Q_PROPERTY(Shape mShape)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString type READ type WRITE setType)
    Q_PROPERTY(qreal x READ x WRITE setX)
    Q_PROPERTY(qreal y READ y WRITE setY)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal width READ width WRITE setWidth)
    Q_PROPERTY(qreal height READ height WRITE setHeight)
    Q_PROPERTY(QSizeF size READ size WRITE setSize)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
    Q_PROPERTY(bool visible READ visible WRITE setVisible)
//    Q_PROPERTY(TextData mTextData)
//    Q_PROPERTY(QPolygonF mPolygon)
//    Q_PROPERTY(Cell mCell)
//    Q_PROPERTY(const ObjectTemplate *mObjectTemplate)
    Q_PROPERTY(Tiled::EditableObjectGroup *layer READ layer)
    Q_PROPERTY(Tiled::EditableMap *map READ map)
//    Q_PROPERTY(bool mTemplateBase)
//    Q_PROPERTY(ChangedProperties mChangedProperties)
    Q_PROPERTY(bool readOnly READ isReadOnly)

public:
    EditableMapObject(EditableMap *map,
                      MapObject *mapObject,
                      QObject *parent = nullptr);

    int id() const;
    QString name() const;
    QString type() const;
    qreal x() const;
    qreal y() const;
    QPointF pos() const;
    qreal width() const;
    qreal height() const;
    QSizeF size() const;
    qreal rotation() const;
    bool visible() const;
    EditableObjectGroup *layer() const;
    EditableMap *map();
    bool isReadOnly() const;

    MapObject *mapObject() const;

    void detach();
    void attach(EditableMap *map);

public slots:
    void setName(QString name);
    void setType(QString type);
    void setX(qreal x);
    void setY(qreal y);
    void setPos(QPointF pos);
    void setWidth(qreal width);
    void setHeight(qreal height);
    void setSize(QSizeF size);
    void setRotation(qreal rotation);
    void setVisible(bool visible);

private:
    void setMapObjectProperty(MapObject::Property property, const QVariant &value);

    EditableMap *mMap;
    MapObject *mMapObject;
    std::unique_ptr<MapObject> mDetachedMapObject;
};


inline int EditableMapObject::id() const
{
    return mMapObject->id();
}

inline QString EditableMapObject::name() const
{
    return mMapObject->name();
}

inline QString EditableMapObject::type() const
{
    return mMapObject->type();
}

inline qreal EditableMapObject::x() const
{
    return mMapObject->x();
}

inline qreal EditableMapObject::y() const
{
    return mMapObject->y();
}

inline QPointF EditableMapObject::pos() const
{
    return mMapObject->position();
}

inline qreal EditableMapObject::width() const
{
    return mMapObject->width();
}

inline qreal EditableMapObject::height() const
{
    return mMapObject->height();
}

inline QSizeF EditableMapObject::size() const
{
    return mMapObject->size();
}

inline qreal EditableMapObject::rotation() const
{
    return mMapObject->rotation();
}

inline bool EditableMapObject::visible() const
{
    return mMapObject->isVisible();
}

inline EditableMap *EditableMapObject::map()
{
    return mMap;
}

inline MapObject *EditableMapObject::mapObject() const
{
    return mMapObject;
}

inline void EditableMapObject::setX(qreal x)
{
    setPos(QPointF(x, y()));
}

inline void EditableMapObject::setY(qreal y)
{
    setPos(QPointF(x(), y));
}

inline void EditableMapObject::setWidth(qreal width)
{
    setSize(QSizeF(width, height()));
}

inline void EditableMapObject::setHeight(qreal height)
{
    setSize(QSizeF(width(), height));
}

} // namespace Tiled
