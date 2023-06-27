/*
 * mapobject.cpp
 * Copyright 2008-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "maprenderer.h"
#include "objectgroup.h"
#include "objecttemplate.h"
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


MapObject::MapObject(const QString &name,
                     const QString &className,
                     const QPointF &pos,
                     const QSizeF &size)
    : Object(MapObjectType, className)
    , mName(name)
    , mPos(pos)
    , mSize(size)
{
}

int MapObject::index() const
{
    if (mObjectGroup)
        return mObjectGroup->objects().indexOf(const_cast<MapObject*>(this));
    return -1;
}

/**
 * Returns the affective class of this object. This may be the class of its
 * tile, if the object does not have a class set explicitly.
 */
const QString &MapObject::effectiveClassName() const
{
    if (className().isEmpty()) {
        if (const MapObject *base = templateObject())
            return base->effectiveClassName();

        if (const Tile *tile = mCell.tile())
            return tile->className();
    }

    return className();
}

/**
 * Sets the text data associated with this object.
 */
void MapObject::setTextData(const TextData &textData)
{
    mTextData = textData;
}

static void align(QRectF &r, Alignment alignment)
{
    r.translate(-alignmentOffset(r.size(), alignment));
}

/**
 * Shortcut to getting a QRectF from position() and size() that uses cell tile
 * if present.
 *
 * \deprecated See problems in comment. Try to use \a screenBounds instead.
 */
QRectF MapObject::boundsUseTile() const
{
    // FIXME: This does not deal with tile offset, rotation, isometric
    // projection and polygons.
    QRectF b = bounds();
    align(b, alignment());
    return b;
}

/**
 * Returns the bounds of the object in screen space when using the given
 * \a renderer. Does not take into account rotation!
 *
 * This is slightly different from the bounds that should be used when
 * rendering the object, which are returned by the MapRenderer::boundingRect
 * function.
 *
 * \todo Look into unduplicating this code, which is also present in
 * objectselectiontool.cpp in very similar form (objectBounds).
 */
QRectF MapObject::screenBounds(const MapRenderer &renderer) const
{
    if (!mCell.isEmpty()) {
        // Tile objects can have a tile offset, which is scaled along with the image
        QSizeF imgSize;
        QPoint tileOffset;

        if (const Tile *tile = mCell.tile()) {
            imgSize = tile->size();
            tileOffset = tile->offset();
        } else {
            imgSize = size();
        }

        const QPointF position = renderer.pixelToScreenCoords(mPos);
        const QSizeF objectSize = size();
        const qreal scaleX = imgSize.width() > 0 ? objectSize.width() / imgSize.width() : 0;
        const qreal scaleY = imgSize.height() > 0 ? objectSize.height() / imgSize.height() : 0;

        QRectF bounds(position.x() + (tileOffset.x() * scaleX),
                      position.y() + (tileOffset.y() * scaleY),
                      objectSize.width(),
                      objectSize.height());

        align(bounds, alignment(renderer.map()));

        return bounds;
    } else {
        switch (mShape) {
        case MapObject::Ellipse:
        case MapObject::Rectangle: {
            QRectF bounds(this->bounds());
            align(bounds, alignment(renderer.map()));
            QPolygonF screenPolygon = renderer.pixelToScreenCoords(bounds);
            return screenPolygon.boundingRect();
        }
        case MapObject::Point:
            return renderer.shape(this).boundingRect();
        case MapObject::Polygon:
        case MapObject::Polyline: {
            // Alignment is irrelevant for polygon objects since they have no size
            const QPolygonF polygon = mPolygon.translated(mPos);
            QPolygonF screenPolygon = renderer.pixelToScreenCoords(polygon);
            return screenPolygon.boundingRect();
        }
        case MapObject::Text:
            return renderer.boundingRect(this);
        }
    }

    return QRectF();
}

QPainterPath MapObject::tileObjectShape(const Map *map) const
{
    const Tile *tile = mCell.tile();
    const QSize tileSize = tile ? tile->size() : QSize(0, 0);

    if (!tile || tileSize.isEmpty()) {
        QPainterPath path;
        path.addRect(QRectF(-alignmentOffset(mSize, alignment(map)), mSize));
        return path;
    }

    QTransform transform;

    const QPointF offset = -alignmentOffset(mSize, alignment(map));
    transform.translate(offset.x(), offset.y());
    transform.scale(mSize.width() / tileSize.width(),
                    mSize.height() / tileSize.height());

    const QPointF tileOffset = tile->offset();
    transform.translate(tileOffset.x(), tileOffset.y());

    if (mCell.flippedHorizontally() || mCell.flippedVertically()) {
        transform.translate(tileSize.width() / 2, tileSize.height() / 2);
        transform.scale(mCell.flippedHorizontally() ? -1 : 1,
                        mCell.flippedVertically() ? -1 : 1);
        transform.translate(-tileSize.width() / 2, -tileSize.height() / 2);
    }

    // It might make sense to cache the transformed shape, but this is
    // non-trivial due to the many factors affecting it.
    return transform.map(tile->imageShape());
}

Map *MapObject::map() const
{
    return mObjectGroup ? mObjectGroup->map() : nullptr;
}

/*
 * Returns the effective alignment for this object on the given \a map.
 *
 * By default, non-tile objects have top-left alignment, while tile objects
 * have bottom-left alignment on orthogonal maps and bottom-center alignment
 * on isometric maps.
 *
 * For tile objects, the default alignment can be overridden by setting an
 * alignment on the tileset.
 */
Alignment MapObject::alignment(const Map *map) const
{
    Alignment alignment = Unspecified;

    if (Tileset *tileset = mCell.tileset())
        alignment = tileset->objectAlignment();

    if (!map && mObjectGroup)
        map = mObjectGroup->map();

    if (alignment == Unspecified) {
        if (mCell.isEmpty())
            return TopLeft;
        else if (map && map->orientation() == Map::Isometric)
            return Bottom;

        return BottomLeft;
    }

    return alignment;
}

/**
 * A helper function to determine the color of a map object. The color is
 * determined first of all by the object class, and otherwise by the group
 * that the object is in. If still no color is defined, it defaults to
 * gray.
 */
MapObjectColors MapObject::effectiveColors() const
{
    MapObjectColors colors;
    bool drawFill = true;

    if (auto classType = Object::propertyTypes().findClassFor(effectiveClassName(), *this)) {
        colors.main = classType->color;
        drawFill = classType->drawFill;
    } else if (mObjectGroup && mObjectGroup->color().isValid()) {
        colors.main = mObjectGroup->color();
    } else {
        colors.main = Qt::gray;
    }

    if (drawFill) {
        colors.fill = colors.main;
        colors.fill.setAlpha(50);
    }

    return colors;
}

QVariant MapObject::mapObjectProperty(Property property) const
{
    switch (property) {
    case NameProperty:          return mName;
    case VisibleProperty:       return mVisible;
    case TextProperty:          return mTextData.text;
    case TextFontProperty:      return mTextData.font;
    case TextAlignmentProperty: return QVariant::fromValue(mTextData.alignment);
    case TextWordWrapProperty:  return mTextData.wordWrap;
    case TextColorProperty:     return mTextData.color;
    case PositionProperty:      return mPos;
    case SizeProperty:          return mSize;
    case RotationProperty:      return mRotation;
    case CellProperty:          Q_ASSERT(false); break;
    case ShapeProperty:         return mShape;
    case TemplateProperty:      Q_ASSERT(false); break;
    case CustomProperties:      Q_ASSERT(false); break;
    case AllProperties:         Q_ASSERT(false); break;
    }
    return QVariant();
}

void MapObject::setMapObjectProperty(Property property, const QVariant &value)
{
    switch (property) {
    case NameProperty:          setName(value.toString()); break;
    case VisibleProperty:       setVisible(value.toBool()); break;
    case TextProperty:          mTextData.text = value.toString(); break;
    case TextFontProperty:      mTextData.font = value.value<QFont>(); break;
    case TextAlignmentProperty: mTextData.alignment = value.value<Qt::Alignment>(); break;
    case TextWordWrapProperty:  mTextData.wordWrap = value.toBool(); break;
    case TextColorProperty:     mTextData.color = value.value<QColor>(); break;
    case PositionProperty:      setPosition(value.toPointF()); break;
    case SizeProperty:          setSize(value.toSizeF()); break;
    case RotationProperty:      setRotation(value.toReal()); break;
    case CellProperty:          Q_ASSERT(false); break;
    case ShapeProperty:         setShape(value.value<Shape>()); break;
    case TemplateProperty:      Q_ASSERT(false); break;
    case CustomProperties:      Q_ASSERT(false); break;
    case AllProperties:         Q_ASSERT(false); break;
    }
}

/**
 * Flip this object in the given \a direction around the given \a origin in
 * screen coordinates. This doesn't change the size of the object.
 */
void MapObject::flip(FlipDirection direction, const QPointF &origin)
{
    if (!mCell.isEmpty() || shape() == Text) {
        flipInScreenCoordinates(direction, origin);
    } else {
        const auto renderer = MapRenderer::create(map());
        const QPointF pixelOrigin = renderer->screenToPixelCoords(origin);

        flipInPixelCoordinates(direction, pixelOrigin);
    }

    setRotation(-rotation());
}

/**
 * Returns a duplicate of this object. The caller is responsible for the
 * ownership of this newly created object.
 */
MapObject *MapObject::clone() const
{
    MapObject *o = new MapObject(mName, className(), mPos, mSize);
    o->setId(mId);
    o->setProperties(properties());
    o->setTextData(mTextData);
    o->setPolygon(mPolygon);
    o->setShape(mShape);
    o->setCell(mCell);
    o->setRotation(mRotation);
    o->setVisible(mVisible);
    o->setChangedProperties(mChangedProperties);
    o->setObjectTemplate(mObjectTemplate);
    return o;
}

void MapObject::copyPropertiesFrom(const MapObject *object)
{
    setName(object->name());
    setSize(object->size());
    setTextData(object->textData());
    setPolygon(object->polygon());
    setShape(object->shape());
    setCell(object->cell());
    setRotation(object->rotation());
    setVisible(object->isVisible());
    setProperties(object->properties());
    setChangedProperties(object->changedProperties());
    setObjectTemplate(object->objectTemplate());
}

const MapObject *MapObject::templateObject() const
{
    if (mObjectTemplate)
        return mObjectTemplate->object();
    return nullptr;
}

void MapObject::syncWithTemplate()
{
    const MapObject *base = templateObject();
    if (!base)
        return;

    if (!propertyChanged(MapObject::NameProperty))
        setName(base->name());

    if (!propertyChanged(MapObject::SizeProperty))
        setSize(base->size());

    if (!propertyChanged(MapObject::TextProperty))
        setTextData(base->textData());

    if (!propertyChanged(MapObject::ShapeProperty)) {
        setShape(base->shape());
        setPolygon(base->polygon());
    }

    if (!propertyChanged(MapObject::CellProperty))
        setCell(base->cell());

    if (!propertyChanged(MapObject::RotationProperty))
        setRotation(base->rotation());

    if (!propertyChanged(MapObject::VisibleProperty))
        setVisible(base->isVisible());
}

void MapObject::detachFromTemplate()
{
    // Can't detach when template object not loaded
    const MapObject *base = templateObject();
    if (!base)
        return;

    // All non-overridden properties are already synchronized, so we only need
    // to merge the class and the custom properties.
    if (className().isEmpty())
        setClassName(base->className());

    Properties newProperties = base->properties();
    Tiled::mergeProperties(newProperties, properties());
    setProperties(newProperties);

    setObjectTemplate(nullptr);
}

/**
 * Tile and text objects are flipped in screen coordinate space. Flipping tile
 * objects also flips their image.
 */
void MapObject::flipInScreenCoordinates(FlipDirection direction, const QPointF &screenOrigin)
{
    const auto renderer = MapRenderer::create(map());
    const QPointF screenPos = renderer->pixelToScreenCoords(position());

    QTransform rotationTransform;
    rotationTransform.rotate(rotation());
    QPointF topLeftScreenPos = screenPos + rotationTransform.map(-alignmentOffset(size(), alignment()));
    QPointF newTopLeftScreenPos = topLeftScreenPos;

    const Alignment flippedAlignment = flipAlignment(alignment(), direction);
    QPointF flippedAlignmentOffset = -alignmentOffset(size(), flippedAlignment);

    if (direction == FlipHorizontally) {
        newTopLeftScreenPos.rx() += 2 * (screenOrigin.x() - topLeftScreenPos.x());
        flippedAlignmentOffset.rx() *= -1;

        if (!mCell.isEmpty())
            mCell.setFlippedHorizontally(!mCell.flippedHorizontally());
    } else { // direction == FlipVertically
        newTopLeftScreenPos.ry() += 2 * (screenOrigin.y() - topLeftScreenPos.y());
        flippedAlignmentOffset.ry() *= -1;

        if (!mCell.isEmpty())
            mCell.setFlippedVertically(!mCell.flippedVertically());
    }

    rotationTransform.reset();
    rotationTransform.rotate(-rotation());
    QPointF newScreenPos = newTopLeftScreenPos - rotationTransform.map(flippedAlignmentOffset);
    QPointF newPos = renderer->screenToPixelCoords(newScreenPos);

    setPosition(newPos);
}

/**
 * Rectangles, ellipses and polygons are flipped in "pixel space", the
 * coordinate space before isometric projection.
 *
 * NOTE: No attempt to handle rotated shapes on isometric maps is made. The
 * expectation is that due to their weird handling, nobody really does that
 * anyway.
 */
void MapObject::flipInPixelCoordinates(FlipDirection direction, const QPointF &pixelOrigin)
{
    QTransform flipTransform;
    if (direction == FlipHorizontally)
        flipTransform.scale(-1, 1);
    else // direction == FlipVertically
        flipTransform.scale(1, -1);

    QTransform flipAroundOriginTransform;
    flipAroundOriginTransform.translate(pixelOrigin.x(), pixelOrigin.y());
    flipAroundOriginTransform = flipTransform * flipAroundOriginTransform;
    flipAroundOriginTransform.translate(-pixelOrigin.x(), -pixelOrigin.y());

    if (!mPolygon.isEmpty()) {
        QTransform polygonToMapTransform;
        polygonToMapTransform.translate(x(), y());
        polygonToMapTransform.rotate(rotation());

        setPosition(flipAroundOriginTransform.map(position()));
        setPolygon(flipTransform.map(mPolygon));
    } else {
        QTransform rotationTransform;
        rotationTransform.rotate(rotation());

        const QPointF newOrigin = alignmentOffset(size(), flipAlignment(alignment(), direction));
        const QPointF newPos = flipAroundOriginTransform.map(position() + rotationTransform.map(newOrigin));
        setPosition(newPos);
    }
}

} // namespace Tiled
