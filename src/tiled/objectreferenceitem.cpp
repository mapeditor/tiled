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
#include <QPainterPath>
#include <QPen>
#include <QVector2D>

#include <array>
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
    static constexpr std::array<QPointF, 4> arrowHead = {
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
    painter->drawPolygon(arrowHead.data(), arrowHead.size());
}


ObjectReferenceItem::ObjectReferenceItem(MapObject *source, QGraphicsItem *parent)
    : ObjectReferenceItem(source, nullptr, parent)
{
}

ObjectReferenceItem::ObjectReferenceItem(MapObject *source,
                                         MapObject *target,
                                         QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mSourceObject(source)
    , mTargetObject(target)
    , mArrowHead(new ArrowHead(this))
{
    setZValue(-0.5); // below labels
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

QPointF ObjectReferenceItem::controlPoint() const
{
    const QPointF mid = (mSourcePos + mTargetPos) / 2.0;
    const QPointF delta = mTargetPos - mSourcePos;
    const QPointF perpendicular(-delta.y(), delta.x()); // rotated 90°, same length as delta
    constexpr qreal curveFactor = 0.25; // seberapa melengkung, silakan disesuaikan
    return mid + perpendicular * curveFactor;
}

void ObjectReferenceItem::updateLineStyle()
{
    prepareGeometryChange();
    updateArrowRotation();
    update();
}

void ObjectReferenceItem::updateArrowRotation()
{
    QPointF tangentOrigin = mSourcePos;

    if (Preferences::instance()->objectReferenceLineStyle() == Preferences::CurvedLine)
        tangentOrigin = controlPoint();

    qreal dx = mTargetPos.x() - tangentOrigin.x();
    qreal dy = mTargetPos.y() - tangentOrigin.y();
    mArrowHead->setRotation(std::atan2(dy, dx) * 180 / M_PI);
}

QRectF ObjectReferenceItem::boundingRect() const
{
    QRectF rect;

    if (Preferences::instance()->objectReferenceLineStyle() == Preferences::CurvedLine) {
        QPainterPath path(mSourcePos);
        path.quadTo(controlPoint(), mTargetPos);
        rect = path.controlPointRect();
    } else {
        rect = QRectF(mSourcePos, mTargetPos).normalized();
    }

    // Margin generous enough to cover the arrow head, the current line
    // width (which the user can change via Preferences), and the small
    // shadow offset drawn in paint().
    const qreal margin = ArrowHead::arrowHeadSize * 2
            + Preferences::instance()->objectLineWidth() + 10;

    return rect.adjusted(-margin, -margin, margin, margin);
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

    auto pen = QPen(mColor, lineWidth, Qt::SolidLine, Qt::RoundCap);
    pen.setCosmetic(true);
    pen.setDashPattern({dashLength, dashLength});

    auto shadowPen = pen;
    shadowPen.setColor(Qt::black);

    painter->setRenderHint(QPainter::Antialiasing);

    const bool curved = Preferences::instance()->objectReferenceLineStyle() == Preferences::CurvedLine;

    if (curved) {
        const QPointF control = controlPoint();

        auto startDir = QVector2D(control - mSourcePos).normalized().toPointF();
        auto endDir = QVector2D(mTargetPos - control).normalized().toPointF();
        auto start = mSourcePos + startDir * ArrowHead::arrowHeadSize / painterScale;
        auto end = mTargetPos - endDir * ArrowHead::arrowHeadSize / painterScale;

        QPainterPath path(start);
        path.quadTo(control, end);

        painter->setPen(shadowPen);
        painter->translate(shadowOffset);
        painter->drawPath(path);
        painter->translate(-shadowOffset);

        painter->setPen(pen);
        painter->drawPath(path);
    } else {
        auto lineLength = static_cast<qreal>(QVector2D(mTargetPos - mSourcePos).length());
        auto dashOffset = lineLength * -0.5 * painterScale / lineWidth;
        pen.setDashOffset(dashOffset);
        shadowPen.setDashOffset(dashOffset);

        auto direction = QVector2D(mTargetPos - mSourcePos).normalized().toPointF();
        auto offset = direction * ArrowHead::arrowHeadSize / painterScale;
        auto start = mSourcePos + offset;
        auto end = mTargetPos - offset;

        painter->setPen(shadowPen);
        painter->drawLine(start + shadowOffset, end + shadowOffset);

        painter->setPen(pen);
        painter->drawLine(start, end);
    }
}

QPointF ObjectReferenceItem::objectCenter(MapObject *object, const MapRenderer &renderer) const
{
    QPointF screenPos = renderer.pixelToScreenCoords(object->position());

    if (object->shape() != MapObject::Point) {
        QRectF bounds = object->screenBounds(renderer);

        // Adjust the bounding box for object rotation
        bounds = rotateAt(screenPos, object->rotation()).mapRect(bounds);
        screenPos = bounds.center();
    }

    if (auto mapScene = qobject_cast<MapScene*>(scene()))
        screenPos += mapScene->absolutePositionForLayer(*object->objectGroup());

    return screenPos;
}

} // namespace Tiled
