/*
 * mapobject.h
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "object.h"
#include "tiled.h"
#include "tilelayer.h"

#include <QFont>
#include <QPolygonF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QTextOption>

namespace Tiled {

class ObjectGroup;
class ObjectTemplate;
class Tile;

struct TILEDSHARED_EXPORT TextData
{
    enum FontAttributes {
        FontFamily  = 0x1,
        FontSize    = 0x2,
        FontStyle   = 0x8
    };

    TextData();

    QString text;
    QFont font;
    QColor color = Qt::black;
    Qt::Alignment alignment = Qt::AlignTop | Qt::AlignLeft;
    bool wordWrap = true;

    int flags() const;
    QTextOption textOption() const;
    QSizeF textSize() const;
};

/**
 * An object on a map. Objects are positioned and scaled using floating point
 * values, ensuring they are not limited to the tile grid. They are suitable
 * for adding any kind of annotation to your maps, as well as free placement of
 * images.
 *
 * Common usages of objects include defining portals, monsters spawn areas,
 * ambient effects, scripted areas, etc.
 */
class TILEDSHARED_EXPORT MapObject : public Object
{
public:
    /**
     * Enumerates the different object shapes. Rectangle is the default shape.
     * When a polygon is set, the shape determines whether it should be
     * interpreted as a filled polygon or a line.
     *
     * Text objects contain arbitrary text, contained withih their rectangle
     * (in screen coordinates).
     */
    enum Shape {
        Rectangle,
        Polygon,
        Polyline,
        Ellipse,
        Text,
        Point,
    };

    /**
     * Can be used to get/set property values using QVariant.
     */
    enum Property {
        NameProperty             = 1 << 0,
        TypeProperty             = 1 << 1,
        VisibleProperty          = 1 << 2,
        TextProperty             = 1 << 3,
        TextFontProperty         = 1 << 4,
        TextAlignmentProperty    = 1 << 5,
        TextWordWrapProperty     = 1 << 6,
        TextColorProperty        = 1 << 7,
        SizeProperty             = 1 << 8,
        RotationProperty         = 1 << 9,
        CellProperty             = 1 << 10,
        ShapeProperty            = 1 << 11
    };

    Q_DECLARE_FLAGS(ChangedProperties, Property)

    MapObject();

    MapObject(const QString &name, const QString &type,
              const QPointF &pos,
              const QSizeF &size);

    int id() const;
    void setId(int id);
    void resetId();

    int index() const;

    const QString &name() const;
    void setName(const QString &name);

    const QString &type() const;
    void setType(const QString &type);

    const QString &effectiveType() const;

    const QPointF &position() const;
    void setPosition(const QPointF &pos);

    qreal x() const;
    void setX(qreal x);

    qreal y() const;
    void setY(qreal y);

    const QSizeF &size() const;
    void setSize(const QSizeF &size);
    void setSize(qreal width, qreal height);

    qreal width() const;
    void setWidth(qreal width);

    qreal height() const;
    void setHeight(qreal height);

    void setBounds(const QRectF &bounds);

    const TextData &textData() const;
    void setTextData(const TextData &textData);

    const QPolygonF &polygon() const;
    void setPolygon(const QPolygonF &polygon);

    Shape shape() const;
    void setShape(Shape shape);

    bool hasDimensions() const;
    bool canRotate() const;
    bool isTileObject() const;

    QRectF bounds() const;
    QRectF boundsUseTile() const;

    const Cell &cell() const;
    void setCell(const Cell &cell);

    const ObjectTemplate *objectTemplate() const;
    void setObjectTemplate(const ObjectTemplate *objectTemplate);

    ObjectGroup *objectGroup() const;
    void setObjectGroup(ObjectGroup *objectGroup);

    qreal rotation() const;
    void setRotation(qreal rotation);

    Alignment alignment() const;

    bool isVisible() const;
    void setVisible(bool visible);

    QVariant mapObjectProperty(Property property) const;
    void setMapObjectProperty(Property property, const QVariant &value);

    void setChangedProperties(ChangedProperties changedProperties);
    MapObject::ChangedProperties changedProperties() const;

    void setPropertyChanged(Property property, bool state = true);
    bool propertyChanged(Property property) const;

    void flip(FlipDirection direction, const QPointF &origin);

    MapObject *clone() const;
    void copyPropertiesFrom(const MapObject *object);

    const MapObject *templateObject() const;

    void syncWithTemplate();

    bool isTemplateInstance() const;

    bool isTemplateBase() const;
    void markAsTemplateBase();

private:
    void flipRectObject(const QTransform &flipTransform);
    void flipPolygonObject(const QTransform &flipTransform);
    void flipTileObject(const QTransform &flipTransform);

    int mId;
    QString mName;
    QString mType;
    QPointF mPos;
    QSizeF mSize;
    TextData mTextData;
    QPolygonF mPolygon;
    Shape mShape;
    Cell mCell;
    const ObjectTemplate *mObjectTemplate;
    ObjectGroup *mObjectGroup;
    qreal mRotation;
    bool mVisible;
    ChangedProperties mChangedProperties;
    bool mTemplateBase;
};

/**
 * Returns the id of this object. Each object gets an id assigned that is
 * unique for the map the object is on.
 */
inline int MapObject::id() const
{ return mId; }

/**
 * Sets the id of this object.
 */
inline void MapObject::setId(int id)
{ mId = id; }

/**
 * Sets the id back to 0. Mostly used when a new id should be assigned
 * after the object has been cloned.
 */
inline void MapObject::resetId()
{ setId(0); }

/**
 * Returns the name of this object. The name is usually just used for
 * identification of the object in the editor.
 */
inline const QString &MapObject::name() const
{ return mName; }

/**
 * Sets the name of this object.
 */
inline void MapObject::setName(const QString &name)
{ mName = name; }

/**
 * Returns the type of this object. The type usually says something about
 * how the object is meant to be interpreted by the engine.
 */
inline const QString &MapObject::type() const
{ return mType; }

/**
 * Sets the type of this object.
 */
inline void MapObject::setType(const QString &type)
{ mType = type; }

/**
 * Returns the position of this object.
 */
inline const QPointF &MapObject::position() const
{ return mPos; }

/**
 * Sets the position of this object.
 */
inline void MapObject::setPosition(const QPointF &pos)
{ mPos = pos; }

/**
 * Returns the x position of this object.
 */
inline qreal MapObject::x() const
{ return mPos.x(); }

/**
 * Sets the x position of this object.
 */
inline void MapObject::setX(qreal x)
{ mPos.setX(x); }

/**
 * Returns the y position of this object.
 */
inline qreal MapObject::y() const
{ return mPos.y(); }

/**
 * Sets the x position of this object.
 */
inline void MapObject::setY(qreal y)
{ mPos.setY(y); }

/**
 * Returns the size of this object.
 */
inline const QSizeF &MapObject::size() const
{ return mSize; }

/**
 * Sets the size of this object.
 */
inline void MapObject::setSize(const QSizeF &size)
{ mSize = size; }

inline void MapObject::setSize(qreal width, qreal height)
{ setSize(QSizeF(width, height)); }

/**
 * Returns the width of this object.
 */
inline qreal MapObject::width() const
{ return mSize.width(); }

/**
 * Sets the width of this object.
 */
inline void MapObject::setWidth(qreal width)
{ mSize.setWidth(width); }

/**
 * Returns the height of this object.
 */
inline qreal MapObject::height() const
{ return mSize.height(); }

/**
 * Sets the height of this object.
 */
inline void MapObject::setHeight(qreal height)
{ mSize.setHeight(height); }

/**
 * Sets the position and size of this object.
 */
inline void MapObject::setBounds(const QRectF &bounds)
{
    mPos = bounds.topLeft();
    mSize = bounds.size();
}

/**
 * Returns the text associated with this object, when it is a text object.
 */
inline const TextData &MapObject::textData() const
{ return mTextData; }

/**
 * Returns the polygon associated with this object. Returns an empty
 * polygon when no polygon is associated with this object.
 */
inline const QPolygonF &MapObject::polygon() const
{ return mPolygon; }

/**
 * Sets the polygon associated with this object. The polygon is only used
 * when the object shape is set to either Polygon or Polyline.
 *
 * \sa setShape()
 */
inline void MapObject::setPolygon(const QPolygonF &polygon)
{ mPolygon = polygon; }

/**
 * Returns the shape of the object.
 */
inline MapObject::Shape MapObject::shape() const
{ return mShape; }

/**
 * Sets the shape of the object.
 */
inline void MapObject::setShape(MapObject::Shape shape)
{ mShape = shape; }

/**
 * Returns true if this object has a width and height.
 */
inline bool MapObject::hasDimensions() const
{
    switch (mShape) {
        case Polygon:
        case Polyline:
        case Point:
            return false;
        default:
            return true;
    }
}

/**
 * Returns true if this object can be rotated.
 */
inline bool MapObject::canRotate() const
{ return mShape != Point; }

inline bool MapObject::isTileObject() const
{ return !mCell.isEmpty(); }

/**
 * Shortcut to getting a QRectF from position() and size().
 */
inline QRectF MapObject::bounds() const
{ return QRectF(mPos, mSize); }

/**
 * Returns the tile associated with this object.
 */
inline const Cell &MapObject::cell() const
{ return mCell; }

/**
 * Sets the tile that is associated with this object. The object will
 * display as the tile image.
 *
 * \warning The object shape is ignored for tile objects!
 */
inline void MapObject::setCell(const Cell &cell)
{ mCell = cell; }

inline const ObjectTemplate *MapObject::objectTemplate() const
{ return mObjectTemplate; }

inline void MapObject::setObjectTemplate(const ObjectTemplate *objectTemplate)
{ mObjectTemplate = objectTemplate; }

/**
 * Returns the object group this object belongs to.
 */
inline ObjectGroup *MapObject::objectGroup() const
{ return mObjectGroup; }

/**
 * Sets the object group this object belongs to. Should only be called
 * from the ObjectGroup class.
 */
inline void MapObject::setObjectGroup(ObjectGroup *objectGroup)
{ mObjectGroup = objectGroup; }

/**
 * Returns the rotation of the object in degrees clockwise.
 */
inline qreal MapObject::rotation() const
{ return mRotation; }

/**
 * Sets the rotation of the object in degrees clockwise.
 */
inline void MapObject::setRotation(qreal rotation)
{ mRotation = rotation; }

inline bool MapObject::isVisible() const
{ return mVisible; }

inline void MapObject::setVisible(bool visible)
{ mVisible = visible; }

inline void MapObject::setChangedProperties(ChangedProperties changedProperties)
{ mChangedProperties = changedProperties; }

inline MapObject::ChangedProperties MapObject::changedProperties() const
{ return mChangedProperties; }

inline void MapObject::setPropertyChanged(Property property, bool state)
{
    if (state)
        mChangedProperties |= property;
    else
        mChangedProperties &= ~property;
}

inline bool MapObject::propertyChanged(Property property) const
{ return mChangedProperties.testFlag(property); }

inline bool MapObject::isTemplateInstance() const
{ return mObjectTemplate != nullptr; }

inline bool MapObject::isTemplateBase() const
{ return mTemplateBase; }

inline void MapObject::markAsTemplateBase()
{ mTemplateBase = true; }

} // namespace Tiled

#if QT_VERSION < 0x050500
Q_DECLARE_METATYPE(Qt::Alignment)
#endif

Q_DECLARE_METATYPE(Tiled::MapObject*)
