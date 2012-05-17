/*
 * mapobject.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include "object.h"

#include <QPolygonF>
#include <QSizeF>
#include <QString>
#include <QRectF>

namespace Tiled {

class ObjectGroup;
class Tile;

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
     */
    enum Shape {
        Rectangle,
        Polygon,
        Polyline
    };

    /**
     * Default constructor.
     */
    MapObject();

    /**
     * Constructor.
     */
    MapObject(const QString &name, const QString &type,
              const QPointF &pos,
              const QSizeF &size);

    /**
     * Destructor.
     */
    ~MapObject() {}

    /**
     * Returns the name of this object. The name is usually just used for
     * identification of the object in the editor.
     */
    const QString &name() const { return mName; }

    /**
     * Sets the name of this object.
     */
    void setName(const QString &name) { mName = name; }

    /**
     * Returns the type of this object. The type usually says something about
     * how the object is meant to be interpreted by the engine.
     */
    const QString &type() const { return mType; }

    /**
     * Sets the type of this object.
     */
    void setType(const QString &type) { mType = type; }

    /**
     * Returns the position of this object.
     */
    const QPointF &position() const { return mPos; }

    /**
     * Sets the position of this object.
     */
    void setPosition(const QPointF &pos) { mPos = pos; }

    /**
     * Returns the x position of this object.
     */
    qreal x() const { return mPos.x(); }

    /**
     * Sets the x position of this object.
     */
    void setX(qreal x) { mPos.setX(x); }

    /**
     * Returns the y position of this object.
     */
    qreal y() const { return mPos.y(); }

    /**
     * Sets the x position of this object.
     */
    void setY(qreal y) { mPos.setY(y); }

    /**
     * Returns the size of this object.
     */
    const QSizeF &size() const { return mSize; }

    /**
     * Sets the size of this object.
     */
    void setSize(const QSizeF &size) { mSize = size; }

    void setSize(qreal width, qreal height)
    { setSize(QSizeF(width, height)); }

    /**
     * Returns the width of this object.
     */
    qreal width() const { return mSize.width(); }

    /**
     * Sets the width of this object.
     */
    void setWidth(qreal width) { mSize.setWidth(width); }

    /**
     * Returns the height of this object.
     */
    qreal height() const { return mSize.height(); }

    /**
     * Sets the height of this object.
     */
    void setHeight(qreal height) { mSize.setHeight(height); }

    /**
     * Sets the polygon associated with this object. The polygon is only used
     * when the object shape is set to either Polygon or Polyline.
     *
     * \sa setShape()
     */
    void setPolygon(const QPolygonF &polygon) { mPolygon = polygon; }

    /**
     * Returns the polygon associated with this object. Returns an empty
     * polygon when no polygon is associated with this object.
     */
    const QPolygonF &polygon() const { return mPolygon; }

    /**
     * Sets the shape of the object.
     */
    void setShape(Shape shape) { mShape = shape; }

    /**
     * Returns the shape of the object.
     */
    Shape shape() const { return mShape; }

    /**
     * Shortcut to getting a QRectF from position() and size().
     */
    QRectF bounds() const { return QRectF(mPos, mSize); }

    /**
     * Sets the tile that is associated with this object. The object will
     * display as the tile image.
     *
     * \warning The object shape is ignored for tile objects!
     */
    void setTile(Tile *tile) { mTile = tile; }

    /**
     * Returns the tile associated with this object.
     */
    Tile *tile() const { return mTile; }

    /**
     * Returns the object group this object belongs to.
     */
    ObjectGroup *objectGroup() const { return mObjectGroup; }

    /**
     * Sets the object group this object belongs to. Should only be called
     * from the ObjectGroup class.
     */
    void setObjectGroup(ObjectGroup *objectGroup)
    { mObjectGroup = objectGroup; }

    /**
     * Returns a duplicate of this object. The caller is responsible for the
     * ownership of this newly created object.
     */
    MapObject *clone() const;

    bool isVisible() const { return mVisible; }
    void setVisible(bool visible) { mVisible = visible; }

private:
    QString mName;
    QString mType;
    QPointF mPos;
    QSizeF mSize;
    QPolygonF mPolygon;
    Shape mShape;
    Tile *mTile;
    ObjectGroup *mObjectGroup;
    bool mVisible;
};

} // namespace Tiled

#endif // MAPOBJECT_H
