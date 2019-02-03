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

#include "editableobject.h"
#include "mapobject.h"

namespace Tiled {

class EditableMap;
class EditableObjectGroup;

class EditableMapObject : public EditableObject
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
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
//    Q_PROPERTY(TextData mTextData)
//    Q_PROPERTY(QPolygonF mPolygon)
//    Q_PROPERTY(Cell mCell)
//    Q_PROPERTY(const ObjectTemplate *mObjectTemplate)
    Q_PROPERTY(Tiled::EditableObjectGroup *layer READ layer)
    Q_PROPERTY(Tiled::EditableMap *map READ map)
//    Q_PROPERTY(bool mTemplateBase)
//    Q_PROPERTY(ChangedProperties mChangedProperties)

public:
    Q_INVOKABLE explicit EditableMapObject(const QString &name = QString(),
                                           QObject *parent = nullptr);

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
    bool isVisible() const;
    bool isSelected() const;
    EditableObjectGroup *layer() const;
    EditableMap *map() const;

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
    void setSelected(bool selected);

private:
    void setMapObjectProperty(MapObject::Property property, const QVariant &value);

    std::unique_ptr<MapObject> mDetachedMapObject;
};


inline int EditableMapObject::id() const
{
    return mapObject()->id();
}

inline QString EditableMapObject::name() const
{
    return mapObject()->name();
}

inline QString EditableMapObject::type() const
{
    return mapObject()->type();
}

inline qreal EditableMapObject::x() const
{
    return mapObject()->x();
}

inline qreal EditableMapObject::y() const
{
    return mapObject()->y();
}

inline QPointF EditableMapObject::pos() const
{
    return mapObject()->position();
}

inline qreal EditableMapObject::width() const
{
    return mapObject()->width();
}

inline qreal EditableMapObject::height() const
{
    return mapObject()->height();
}

inline QSizeF EditableMapObject::size() const
{
    return mapObject()->size();
}

inline qreal EditableMapObject::rotation() const
{
    return mapObject()->rotation();
}

inline bool EditableMapObject::isVisible() const
{
    return mapObject()->isVisible();
}

inline MapObject *EditableMapObject::mapObject() const
{
    return static_cast<MapObject*>(object());
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
