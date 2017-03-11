/*
 * mapobject.cpp
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2017, Klimov Viktor <vitek.fomino@bk.ru>
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

#include "mapobject.h"

#include "map.h"
#include "objectgroup.h"
#include "tile.h"

#include <QFontMetricsF>
#include <qmath.h>

namespace Tiled {

TextData::TextData()
    : font(QStringLiteral("sans-serif"))
{
    font.setPixelSize(16);
}

int TextData::flags() const
{
    return wordWrap ? (alignment | Qt::TextWordWrap) : alignment;
}

QTextOption TextData::textOption() const
{
    QTextOption option(alignment);

    if (wordWrap)
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    else
        option.setWrapMode(QTextOption::ManualWrap);

    return option;
}

/**
 * Returns the size of the text when drawn without wrapping.
 */
QSizeF TextData::textSize() const
{
    QFontMetricsF fontMetrics(font);
    return fontMetrics.size(0, text);
}


MapObject::MapObject():
    Object(MapObjectType),
    mId(0),
    mSize(0, 0),
    mShape(Rectangle),
    mObjectGroup(nullptr),
    mRotation(0.0f),
    mVisible(true)
{
}

MapObject::MapObject(const QString &name, const QString &type,
                     const QPointF &pos,
                     const QSizeF &size):
    Object(MapObjectType),
    mId(0),
    mName(name),
    mType(type),
    mPos(pos),
    mSize(size),
    mShape(Rectangle),
    mObjectGroup(nullptr),
    mRotation(0.0f),
    mVisible(true)
{
}

/**
 * Returns the affective type of this object. This may be the type of its tile,
 * if the object does not have a type set explicitly.
 */
const QString &MapObject::effectiveType() const
{
    if (mType.isEmpty())
        if (const Tile *tile = mCell.tile())
            return tile->type();

    return mType;
}

/**
 * Sets the text data associated with this object.
 */
void MapObject::setTextData(const TextData &textData)
{
    mTextData = textData;
}

/**
 * Shortcut to getting a QRectF from position() and size() that uses cell tile if present.
 */
QRectF MapObject::boundsUseTile() const
{
    // FIXME: This is outdated code:
    // * It does not take into account that a tile object can be scaled.
    // * It neglects that origin is not the same in orthogonal and isometric
    //   maps (see MapObject::alignment).
    // * It does not deal with rotation.

    if (const Tile *tile = mCell.tile()) {
        // Using the tile for determing boundary
        // Note the position given is the bottom-left corner so correct for that
        return QRectF(QPointF(mPos.x(),
                              mPos.y() - tile->height()),
                      tile->size());
    }

    // No tile so just use regular bounds
    return bounds();
}

/*
 * This is somewhat of a workaround for dealing with the ways different objects
 * align.
 *
 * Traditional rectangle objects have top-left alignment.
 * Tile objects have bottom-left alignment on orthogonal maps, but
 * bottom-center alignment on isometric maps.
 *
 * Eventually, the object alignment should probably be configurable. For
 * backwards compatibility, it will need to be configurable on a per-object
 * level.
 */
Alignment MapObject::alignment() const
{
    if (mCell.isEmpty()) {
        return TopLeft;
    } else if (mObjectGroup) {
        if (Map *map = mObjectGroup->map())
            if (map->orientation() == Map::Isometric)
                return Bottom;
    }
    return BottomLeft;
}

QVariant MapObject::mapObjectProperty(Property property) const
{
    switch (property) {
    case NameProperty:          return mName;
    case TypeProperty:          return mType;
    case VisibleProperty:       return mVisible;
    case TextProperty:          return mTextData.text;
    case TextFontProperty:      return mTextData.font;
    case TextAlignmentProperty: return QVariant::fromValue(mTextData.alignment);
    case TextWordWrapProperty:  return mTextData.wordWrap;
    case TextColorProperty:     return mTextData.color;
    }
    return QVariant();
}

void MapObject::setMapObjectProperty(Property property, const QVariant &value)
{
    switch (property) {
    case NameProperty:          mName = value.toString(); break;
    case TypeProperty:          mType = value.toString(); break;
    case VisibleProperty:       mVisible = value.toBool(); break;
    case TextProperty:          mTextData.text = value.toString(); break;
    case TextFontProperty:
        mTextData.font = value.value<QFont>();
        break;
    case TextAlignmentProperty: mTextData.alignment = value.value<Qt::Alignment>(); break;
    case TextWordWrapProperty:  mTextData.wordWrap = value.toBool(); break;
    case TextColorProperty:     mTextData.color = value.value<QColor>(); break;
    }
}

/**
 * Flip this object in the given \a direction . This doesn't change the size
 * of the object.
 */
void MapObject::flip(FlipDirection direction, const QPointF &origin)
{
    if(!mCell.isEmpty())
        flipTileObject(direction, origin);
    else if(!mPolygon.isEmpty())
        flipPolygonObject(direction, origin);
    else
        flipRectObject(direction, origin);
}

/**
 * Returns a duplicate of this object. The caller is responsible for the
 * ownership of this newly created object.
 */
MapObject *MapObject::clone() const
{
    MapObject *o = new MapObject(mName, mType, mPos, mSize);
    o->setId(mId);
    o->setProperties(properties());
    o->setTextData(mTextData);
    o->setPolygon(mPolygon);
    o->setShape(mShape);
    o->setCell(mCell);
    o->setRotation(mRotation);
    o->setVisible(mVisible);
    return o;
}

void MapObject::flipRectObject(FlipDirection direction, const QPointF &origin)
{
    QTransform flipTransform;
    flipTransform.translate(origin.x(), origin.y());
    if(direction == FlipHorizontally)
        flipTransform.scale(-1, 1);
    else //direction == FlipVertically
        flipTransform.scale(1, -1);
    flipTransform.translate(-origin.x(), -origin.y());

    // point 1 is position
    // 0-----1
    // |     |
    // 3-----2


    QPointF points[4];
    points[0] = position();
    points[1] = QPointF(cos(qDegreesToRadians(rotation())) * width() + x(),
                        sin(qDegreesToRadians(rotation())) * width() + y());

    points[3] = QPointF(cos(qDegreesToRadians(rotation() + 90)) * height() + x(),
                        sin(qDegreesToRadians(rotation() + 90)) * height() + y());

    points[2] = QPointF(cos(qDegreesToRadians(rotation())) * width() + points[3].x(),
            sin(qDegreesToRadians(rotation())) * width() + points[3].y());


    QPointF newPointTwo = flipTransform.map(points[2]);
    QPointF newPointThree = flipTransform.map(points[3]);
    qreal newRotation = -QLineF(newPointThree, newPointTwo).angle();

    setPosition(newPointThree);
    setRotation(newRotation);
}

void MapObject::flipPolygonObject(FlipDirection direction, const QPointF &origin)
{
    QTransform flipTransform;
    flipTransform.translate(origin.x(), origin.y());
    if(direction == FlipHorizontally)
        flipTransform.scale(-1, 1);
    else //direction == FlipVertically
        flipTransform.scale(1, -1);
    flipTransform.translate(-origin.x(), -origin.y());

    // point 1 is position
    // 0-----1
    // |     |
    // 3-----2

    QRectF polygonRect = mPolygon.boundingRect();
    qreal polyWidth = polygonRect.width();
    qreal polyHeight = polygonRect.height();

    QPointF points[4];
    points[0] = position();
    points[1] = QPointF(cos(qDegreesToRadians(rotation())) * polyWidth + x(),
                        sin(qDegreesToRadians(rotation())) * polyWidth + y());

    points[3] = QPointF(cos(qDegreesToRadians(rotation() + 90)) * polyHeight + x(),
                            sin(qDegreesToRadians(rotation() + 90)) * polyHeight + y());

    points[2] = QPointF(cos(qDegreesToRadians(rotation())) * polyWidth + points[3].x(),
                        sin(qDegreesToRadians(rotation())) * polyWidth + points[3].y());



    QPointF newPointTwo = flipTransform.map(points[2]);
    QPointF newPointThree = flipTransform.map(points[3]);
    qreal newRotation = -QLineF(newPointThree, newPointTwo).angle();
    setPosition(newPointThree);
    setRotation(newRotation);

    //flip polygon
    QPointF polygonCenter = mPolygon.boundingRect().center();
    QTransform flipPolygonTransform;
    flipPolygonTransform.translate(polygonCenter.x(), polygonCenter.y());
    if (direction != FlipHorizontally)
        flipPolygonTransform.scale(-1, 1);
    else //direction == FlipVertically
        flipPolygonTransform.scale(1, -1);
    flipPolygonTransform.translate(-polygonCenter.x(), -polygonCenter.y());

    mPolygon = flipPolygonTransform.map(mPolygon);
}

void MapObject::flipTileObject(FlipDirection direction, const QPointF &origin)
{
    if (direction == FlipHorizontally)
        mCell.setFlippedHorizontally(!mCell.flippedHorizontally());
    else // direction == FlipVertically)
        mCell.setFlippedVertically(!mCell.flippedVertically());


    QTransform flipTransform;
    flipTransform.translate(origin.x(), origin.y());
    if(direction == FlipHorizontally)
        flipTransform.scale(-1, 1);
    else //direction == FlipVertically
        flipTransform.scale(1, -1);
    flipTransform.translate(-origin.x(), -origin.y());

    // point 1 is position
    // 3-----2
    // |     |
    // 0-----1

    QPointF points[4];
    points[0] = position();
    points[1] = QPointF(cos(qDegreesToRadians(rotation())) * width() + x(),
                        sin(qDegreesToRadians(rotation())) * width() + y());

    points[3] = QPointF(cos(qDegreesToRadians(rotation() - 90)) * height() + x(),
                        sin(qDegreesToRadians(rotation() - 90)) * height() + y());

    points[2] = QPointF(cos(qDegreesToRadians(rotation())) * width() + points[3].x(),
            sin(qDegreesToRadians(rotation())) * width() + points[3].y());


    QPointF newPointTwo = flipTransform.map(points[2]);
    QPointF newPointThree = flipTransform.map(points[3]);
    qreal newRotation = -QLineF(newPointThree, newPointTwo).angle();

    setPosition(newPointThree);
    setRotation(newRotation);
}

} // namespace Tiled
