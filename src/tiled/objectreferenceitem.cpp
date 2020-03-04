/*
 * objectreferenceitem.cpp
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "objectreferenceitem.h"

#include "geometry.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "objectselectionitem.h"
#include "preferences.h"
#include "utils.h"

#include <QPainter>
#include <QPen>
#include <QVector2D>

#include <cmath>

namespace Tiled {

class ArrowHead : public QGraphicsItem
{
public:
    static constexpr qreal arrowHeadSize = 7.0;

    ArrowHead(QGraphicsItem *parent)
        : QGraphicsItem(parent)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations);
    }

    void setColor(const QColor &color) { mColor = color; update(); }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    QColor mColor;
};

QRectF ArrowHead::boundingRect() const
{
    return Utils::dpiScaled(QRectF(-2 * arrowHeadSize,
                                   -arrowHeadSize,
                                   2 * arrowHeadSize,
                                   2 * arrowHeadSize));
}

void ArrowHead::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    constexpr QPointF arrowHead[4] = {
        QPointF(0.0, 0.0),
        QPointF(-2 * arrowHeadSize, arrowHeadSize),
        QPointF(-1.5 * arrowHeadSize, 0.0),
        QPointF(-2 * arrowHeadSize, -arrowHeadSize)
    };

    const qreal dpiScale = Utils::defaultDpiScale();
    painter->scale(dpiScale, dpiScale);

    QPen arrowOutline(Qt::black);
    arrowOutline.setCosmetic(true);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(mColor);
    painter->setPen(arrowOutline);
    painter->drawPolygon(arrowHead, 4);
}


ObjectReferenceItem::ObjectReferenceItem(MapObject *source, QGraphicsItem *parent)
    : ObjectReferenceItem(source, nullptr, QString(), parent)
{
}

ObjectReferenceItem::ObjectReferenceItem(MapObject *source,
                                         MapObject *target,
                                         const QString &property,
                                         QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mSourceObject(source)
    , mTargetObject(target)
    , mArrowHead(new ArrowHead(this))
    , mProperty(property)
{
    setZValue(-0.5); // below labels but above hover
    updateColor();
}

void ObjectReferenceItem::setTargetPos(const QPointF &pos)
{
    if (mTargetPos == pos)
        return;

    prepareGeometryChange();
    mTargetPos = pos;
    mArrowHead->setPos(mTargetPos);
    update();
    updateArrowRotation();
}

void ObjectReferenceItem::syncWithSourceObject(const MapRenderer &renderer)
{
    const QPointF sourcePos = objectCenter(mSourceObject, renderer);

    if (mSourcePos != sourcePos) {
        prepareGeometryChange();
        mSourcePos = sourcePos;
        update();
        updateArrowRotation();
    }
}

void ObjectReferenceItem::syncWithTargetObject(const MapRenderer &renderer)
{
    if (mTargetObject)
        setTargetPos(objectCenter(mTargetObject, renderer));
    updateColor();  // color is based on target object
}

void ObjectReferenceItem::updateColor()
{
    const QColor color = mTargetObject ? mTargetObject->effectiveColor() : Qt::gray;

    if (mColor != color) {
        mColor = color;
        update();
        mArrowHead->setColor(color);
    }
}

void ObjectReferenceItem::updateArrowRotation()
{
    qreal dx = mTargetPos.x() - mSourcePos.x();
    qreal dy = mTargetPos.y() - mSourcePos.y();
    mArrowHead->setRotation(std::atan2(dy, dx) * 180 / M_PI);
}

QRectF ObjectReferenceItem::boundingRect() const
{
    return QRectF(mSourcePos, mTargetPos).normalized().adjusted(-5, -5, 5, 5);
}

void ObjectReferenceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    qreal painterScale = 1.0;
    if (auto mapScene = qobject_cast<MapScene*>(scene()))
        painterScale = mapScene->mapDocument()->renderer()->painterScale();

    auto lineWidth = Preferences::instance()->objectLineWidth();
    auto shadowDist = (lineWidth == 0 ? 1 : lineWidth) / painterScale;
    auto shadowOffset = QPointF(shadowDist * 0.5, shadowDist * 0.5);
    auto devicePixelRatio = painter->device()->devicePixelRatioF();
    auto dashLength = std::ceil(Utils::dpiScaled(2) * devicePixelRatio);
    auto lineLength = static_cast<qreal>(QVector2D(mTargetPos - mSourcePos).length());
    auto dashOffset = lineLength * -0.5 * painterScale / lineWidth;

    auto pen = QPen(mColor, lineWidth, Qt::SolidLine, Qt::RoundCap);
    pen.setCosmetic(true);
    pen.setDashPattern({dashLength, dashLength});
    pen.setDashOffset(dashOffset);

    auto shadowPen = pen;
    shadowPen.setColor(Qt::black);

    auto direction = QVector2D(mTargetPos - mSourcePos).normalized().toPointF();
    auto offset = direction * ArrowHead::arrowHeadSize / painterScale;
    auto start = mSourcePos + offset;
    auto end = mTargetPos - offset;

    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(shadowPen);
    painter->drawLine(start + shadowOffset, end + shadowOffset);

    painter->setPen(pen);
    painter->drawLine(start, end);
}

QPointF ObjectReferenceItem::objectCenter(MapObject *object, const MapRenderer &renderer)
{
    QPointF screenPos = renderer.pixelToScreenCoords(object->position());

    if (object->shape() != MapObject::Point) {
        QRectF bounds = object->screenBounds(renderer);

        // Adjust the bounding box for object rotation
        bounds = rotateAt(screenPos, object->rotation()).mapRect(bounds);
        screenPos = bounds.center();
    }

    return screenPos + object->objectGroup()->totalOffset();
}

} // namespace Tiled
