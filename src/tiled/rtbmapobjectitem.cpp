/*
 * rtbmapobjectitem.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbmapobjectitem.h"

#include "mapobject.h"
#include "mapdocument.h"
#include "map.h"
#include "objectgroup.h"
#include "rtbmapobject.h"
#include "rtbmapsettings.h"
#include "mapview.h"
#include "zoomable.h"
#include "mapobjectitem.h"
#include "mainwindow.h"
#include "rtbcore.h"

#include <QPainter>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

const QRectF borderRect(0, -32, 32, 32);

// label position
const QRectF centerLabel(0, -32, 32, 32);
const QRectF rightLabel(34, -23, 25, 12);
const QRectF leftLabel(-27, -23, 25, 12);
const QRectF bottomLabel(4, 2, 25, 12);
const QRectF topLabel(4, -46, 25, 12);

// this point set the end of the laser to the end of the target cell
const QPointF laserBeamRightDelta(0, 16);
const QPointF laserBeamLeftDelta(32, 16);
const QPointF laserBeamTopDelta(16, 32);
const QPointF laserBeamBottomDelta(16, 0);
// this point set the start of the laser to the start of the start cell
const QPointF laserBeamRightStartDelta(7, 0);
const QPointF laserBeamLeftStartDelta(-7, 0);
const QPointF laserBeamTopStartDelta(0, -7);
const QPointF laserBeamBottomStartDelta(0, 7);

/**
 * Shared superclass for visualization.
 */
class RTBVisualization : public QGraphicsItem
{
public:
    RTBVisualization(MapObject *mapObject, MapDocument *mapDocument, QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
        , mMapObject(mapObject)
        , mMapDocument(mapDocument)
        , mScale(1)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations);
    }

    QRectF boundingRect() const{ return parentItem()->boundingRect(); }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget);

protected:
    void drawTargetLine(QPainter *painter, MapObject *rootObject, MapObject *targetObject);
    QRectF drawTriggerZone(QPainter *painter, int width, int height, double offsetX = 0, double offsetY = 0);
    void scaleFont(QPainter *painter);

    MapObject *mMapObject;
    MapDocument *mMapDocument;
    QRectF mRect;
    qreal mScale;
};

void RTBVisualization::drawTargetLine(QPainter *painter, MapObject *rootObject, MapObject *targetObject)
{
    QLineF targetLine(parentItem()->mapFromParent(rootObject->boundsUseTile().center()) * mScale, parentItem()->mapFromParent(targetObject->boundsUseTile().center()) * mScale);
    painter->drawLine(targetLine);
}

QRectF RTBVisualization::drawTriggerZone(QPainter *painter, int width, int height, double offsetX, double offsetY)
{
    if(width > 0 && height > 0)
    {
        painter->setPen(QPen(Qt::cyan, 2));
        QPointF topLeft(int(width / 2) * 32, int(height / 2) * 32);
        QSizeF size(width * 32, height * 32);
        QRectF rect(topLeft * mScale, size * mScale);
        // move center of the rect to the center of the object
        rect.moveCenter(parentItem()->mapFromParent(mMapObject->boundsUseTile().center()) * mScale);

        if(offsetX != 0 || offsetY != 0)
            rect.moveCenter(rect.center() + (QPointF(offsetX * 32, offsetY * 32) * mScale));

        painter->drawRect(rect);
        return rect;
    }

    return QRectF();
}

void RTBVisualization::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget)
{
    // paint the selection rectangle if the object is selected
    if (static_cast<MapObjectItem*>(parentItem())->isEditable()) {
        // scale for the zoomlevel
        mScale = static_cast<MapView*>(widget->parent())->zoomable()->scale();

        QPen dashPen(Qt::DashLine);
        dashPen.setColor(Qt::white);
        dashPen.setCosmetic(true);
        dashPen.setDashOffset(qMax(qreal(0), x()));
        painter->setPen(dashPen);
        painter->drawRect(QRectF(QPointF(-2, -34) * mScale, QSize(35, 35) * mScale));
    }
}

void RTBVisualization::scaleFont(QPainter *painter)
{
    QFont font = painter->font();
    font.setPointSize(font.pointSize() * mScale);
    painter->setFont(font);
}

//=============================================================================

class RTBMapObjectValidate : public RTBVisualization
{
public:
    RTBMapObjectValidate(MapObject *mapObject, MapDocument *mapDocument, QGraphicsItem *parent = 0)
        : RTBVisualization(mapObject, mapDocument, parent)
    {
        setZValue(-1000);
    }

    QRectF boundingRect() const{ return QRectF(-2, -34, 36, 36); }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget);

};

void RTBMapObjectValidate::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *widget)
{
    // scale for the zoomlevel
    mScale = static_cast<MapView*>(widget->parent())->zoomable()->scale();

    RTBMapObject *rtbObject = mMapObject->rtbMapObject();

    if(rtbObject->hasError())
    {
        painter->setPen(QPen(Qt::red, 2));
        painter->drawRect(QRectF(borderRect.topLeft() * mScale, borderRect.size() * mScale));
    }
    else if(rtbObject->hasWarning())
    {
        painter->setPen(QPen(RTBMapObject::warningColor(), 2));
        painter->drawRect(QRectF(borderRect.topLeft() * mScale, borderRect.size() * mScale));
    }
}

//=============================================================================

class RTBMapObjectLabel : public RTBVisualization
{
public:
    RTBMapObjectLabel(MapObject *mapObject, MapDocument *mapDocument, QGraphicsItem *parent = 0)
        : RTBVisualization(mapObject, mapDocument, parent)
    {
        int objectType = mapObject->rtbMapObject()->objectType();

        // set position and size of the rect which contains the text
        if(objectType == RTBMapObject::LaserBeam)
        {
            switch (mapObject->cell().tile->id()) {
            case RTBMapObject::LaserBeamLeft:
                mRect = leftLabel;
                break;
            case RTBMapObject::LaserBeamRight:
                mRect = rightLabel;
                break;
            case RTBMapObject::LaserBeamBottom:
                mRect = bottomLabel;
                break;
            case RTBMapObject::LaserBeamTop:
                mRect = topLabel;
                break;
            }
        }
        else
        {
            mRect = centerLabel;
        }
    }

    QRectF boundingRect() const{ return mRect; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget);

private:
    QString intervalOffsetTextValue(RTBMapObject::Objects objectType, int value);
};

void RTBMapObjectLabel::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *widget)
{
    // scale for the zoomlevel
    mScale = static_cast<MapView*>(widget->parent())->zoomable()->scale();
    // create the scaled rect
    QRectF scaledRect(mRect.topLeft() * mScale, mRect.size() * mScale);

    painter->setPen(Qt::white);
    painter->setRenderHint(QPainter::Antialiasing);
    scaleFont(painter);

    RTBMapObject *rtbObject = mMapObject->rtbMapObject();

    switch (rtbObject->objectType()) {
    case RTBMapObject::Button:
    {
        RTBButtonObject *obj = static_cast<RTBButtonObject*>(rtbObject);
        painter->drawText(scaledRect, Qt::AlignCenter, QString::number(obj->beatsActive()));
        break;
    }
    case RTBMapObject::CustomFloorTrap:
    {
        RTBCustomFloorTrap *obj = static_cast<RTBCustomFloorTrap*>(rtbObject);
        painter->drawText(scaledRect, Qt::AlignCenter
                          , RTBCore::intervallOffsetValue(RTBMapObject::CustomFloorTrap, obj->intervalOffset()));
        break;
    }
    case RTBMapObject::MovingFloorTrapSpawner:
    {
        RTBMovingFloorTrapSpawner *obj = static_cast<RTBMovingFloorTrapSpawner*>(rtbObject);
        painter->drawText(scaledRect, Qt::AlignCenter, QString::number(obj->spawnAmount()));
        break;
    }
    case RTBMapObject::ProjectileTurret:
    {
        RTBProjectileTurret *obj = static_cast<RTBProjectileTurret*>(rtbObject);
        painter->drawText(scaledRect, Qt::AlignHCenter | Qt::AlignBottom
                          , RTBCore::intervallOffsetValue(RTBMapObject::ProjectileTurret, obj->intervalOffset()));
        break;
    }
    case RTBMapObject::LaserBeam:
    {
        RTBLaserBeam *obj = static_cast<RTBLaserBeam*>(rtbObject);
        if(obj->beamType() == RTBMapObject::BT2)
            painter->drawText(scaledRect, Qt::AlignCenter
                              , RTBCore::intervallOffsetValue(RTBMapObject::LaserBeam, obj->intervalOffset()));
        break;
    }
    case RTBMapObject::NPCBallSpawner:
    {
        RTBNPCBallSpawner *obj = static_cast<RTBNPCBallSpawner*>(rtbObject);
        painter->drawText(scaledRect, Qt::AlignCenter
                          , RTBCore::intervallOffsetValue(RTBMapObject::NPCBallSpawner, obj->intervalOffset()));
        break;
    }
    default:
        break;
    }
}

//=============================================================================

class RTBVisualizePropHandle : public RTBVisualization
{
public:
    RTBVisualizePropHandle(MapObject *mapObject, MapDocument *mapDocument, QGraphicsItem *parent = 0)
        : RTBVisualization(mapObject, mapDocument, parent)
    {
    }

    QRectF boundingRect() const
    {
        if(mMapDocument && mMapObject)
        {
                if(mMapObject->rtbMapObject()->objectType() == RTBMapObject::ProjectileTurret)
                {
                    return QRectF(- 32, - 64, 96, 96);
                }
                else if(mMapObject->rtbMapObject()->objectType() == RTBMapObject::FloorText)
                {
                    const RTBFloorText *floorText = static_cast<const RTBFloorText*>(mMapObject->rtbMapObject());
                    int minWidth = mFloorTextSize * 2;
                    qreal triggerZoneWidth = floorText->triggerZoneSize().width();
                    qreal width;
                    triggerZoneWidth < minWidth ? width = minWidth : width = triggerZoneWidth;
                    qreal height = floorText->triggerZoneSize().height();

                    QPointF topLeft(int(width / 2) * 32, int(height / 2) * 32);
                    QSizeF size(width * 32, height * 32);
                    QRectF rect(topLeft * mScale, size * mScale);
                    // move center of the rect to the center of the object
                    rect.moveCenter(parentItem()->mapFromParent(mMapObject->boundsUseTile().center()) * mScale);

                    double offsetX = floorText->offsetX();
                    double offsetY = floorText->offsetY();
                    if(offsetX > 0){
                        rect.setRight(rect.right() + offsetX * 32);

                    }else if(offsetX < 0){
                        rect.setLeft(rect.left() + offsetX * 32);
                    }

                    if(offsetY > 0){
                        rect.setBottom(rect.bottom() + offsetY * 32);

                    }else if(offsetY < 0){
                        rect.setTop(rect.top() + offsetY * 32);
                    }

                    return rect;
                }
                else
                {
                    QRectF bounds = mMapObject->boundsUseTile();
                    // refresh the complete map
                    return QRectF(-bounds.topLeft().x(), -bounds.topLeft().y() - 32, mMapDocument->map()->size().width() * 32, mMapDocument->map()->size().height() * 32);
                }
        }
        else return QRectF();

    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget);

    void visualizeButtonProperties(QPainter *painter, RTBButtonObject *rtbObj);
    void visualizeTeleporterProperties(QPainter *painter, RTBTeleporter* rtbObj);
    void visualizeProjectileTurretProperties(QPainter *painter, RTBProjectileTurret* rtbObj);
    void visualizeFloorTextProperties(QPainter *painter, RTBFloorText* rtbObj);
    void visualizeCameraTriggerProperties(QPainter *painter, RTBCameraTrigger *rtbObj);

    void drawDirectionArrow(QPainter *painter, int direction);

private:
    int mFloorTextSize = 4;

};

void RTBVisualizePropHandle::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *widget)
{
    if(!mMapObject || !mMapDocument)
        return;

    MainWindow *mainWindow = static_cast<MainWindow*>(widget->parent()->parent()->parent()->parent()->parent());

    // show only if the object is selected and object layer is selected or showProp ist activated
    if(!mainWindow->showPropVisualization() && (!static_cast<MapObjectItem*>(parentItem())->isEditable()
            || mMapDocument->currentLayerIndex() != RTBMapSettings::ObjectID))
        return;

    painter->setRenderHint(QPainter::Antialiasing);
    setPos(0, 0);
    setRotation(0);

    // scale for the zoomlevel
    mScale = static_cast<MapView*>(widget->parent())->zoomable()->scale();

    RTBMapObject *rtbMapObject = mMapObject->rtbMapObject();

    switch (rtbMapObject->objectType()) {
    case RTBMapObject::Button:
    {
        visualizeButtonProperties(painter, static_cast<RTBButtonObject*>(rtbMapObject));
        break;
    }
    case RTBMapObject::Teleporter:
    {
        visualizeTeleporterProperties(painter, static_cast<RTBTeleporter*>(rtbMapObject));
        break;
    }
    case RTBMapObject::ProjectileTurret:
    {
        visualizeProjectileTurretProperties(painter, static_cast<RTBProjectileTurret*>(rtbMapObject));
        break;
    }
    case RTBMapObject::FloorText:
    {
        visualizeFloorTextProperties(painter, static_cast<RTBFloorText*>(rtbMapObject));
        break;
    }
    case RTBMapObject::CameraTrigger:
    {
        visualizeCameraTriggerProperties(painter, static_cast<RTBCameraTrigger*>(rtbMapObject));
        break;
    }
    default:
        break;
    }

    // needed to refresh if the object is changed
    update();
}

void RTBVisualizePropHandle::visualizeButtonProperties(QPainter *painter, RTBButtonObject *rtbObj)
{
    painter->setPen(QPen(Qt::cyan, 2, Qt::DotLine));

    QList<MapObject *> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    for(MapObject * obj : objects)
    {
        if(rtbObj->containsTarget(QString::number(obj->id())))
        {
            drawTargetLine(painter, mMapObject, obj);
        }
    }
}

void RTBVisualizePropHandle::visualizeTeleporterProperties(QPainter *painter, RTBTeleporter* rtbObj)
{
    painter->setPen(QPen(Qt::cyan, 2, Qt::DotLine));

    QString target = rtbObj->teleporterTarget();

    QList<MapObject *> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    for(MapObject * obj : objects)
    {
        if(target == QString::number(obj->id()))
        {
            drawTargetLine(painter, mMapObject, obj);
        }
    }
}

void RTBVisualizePropHandle::visualizeProjectileTurretProperties(QPainter *painter, RTBProjectileTurret* rtbObj)
{
    painter->setPen(QPen(Qt::red, 1, Qt::SolidLine));

    drawDirectionArrow(painter, rtbObj->shotDirection());
}

void RTBVisualizePropHandle::visualizeFloorTextProperties(QPainter *painter, RTBFloorText* rtbObj)
{
    if(rtbObj->useTrigger()){
        drawTriggerZone(painter, rtbObj->triggerZoneSize().width(), rtbObj->triggerZoneSize().height()
                        , rtbObj->offsetX(), rtbObj->offsetY());
    }

    if(!rtbObj->text().isEmpty())
    {
        painter->setPen(QPen(Qt::white, 1));
        scaleFont(painter);

        // create the scaled rect
        QRectF boundingRect = parentItem()->boundingRect();
        boundingRect.setLeft(boundingRect.left() - (mFloorTextSize * 32));
        boundingRect.setRight(boundingRect.right() + (mFloorTextSize * 32));
        QRectF scaledRect(boundingRect.topLeft() * mScale, boundingRect.size() * mScale);
        painter->drawText(scaledRect, Qt::AlignCenter, rtbObj->text().left(30));
    }
}

void RTBVisualizePropHandle::visualizeCameraTriggerProperties(QPainter *painter, RTBCameraTrigger* rtbObj)
{
    painter->setPen(QPen(Qt::cyan, 2, Qt::DotLine));

    QString target = rtbObj->target();

    QList<MapObject *> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    for(MapObject * obj : objects)
    {
        if(target == QString::number(obj->id()))
        {
            drawTargetLine(painter, mMapObject, obj);
        }
    }

    drawTriggerZone(painter, rtbObj->triggerZoneSize().width(), rtbObj->triggerZoneSize().height());
}

void RTBVisualizePropHandle::drawDirectionArrow(QPainter *painter, int direction)
{
    switch (direction) {
    case RTBMapObject::Left:
    {
        QPointF newPos = parentItem()->mapFromParent(QPointF(mMapObject->boundsUseTile().topLeft()));
        newPos.setY(-16);
        setPos(newPos);
        setRotation(90);
        break;
    }
    case RTBMapObject::Right:
    {
        QPointF newPos = parentItem()->mapFromParent(QPointF(mMapObject->boundsUseTile().topRight()));
        newPos.setY(-16);
        setPos(newPos);
        setRotation(-90);
        break;
    }
    case RTBMapObject::Up:
    {
        QPointF newPos = parentItem()->mapFromParent(QPointF(mMapObject->boundsUseTile().topLeft()));
        newPos.setX(16);
        setPos(newPos);
        setRotation(180);
        break;
    }
    case RTBMapObject::Down:
    {
        QPointF newPos = parentItem()->mapFromParent(QPointF(mMapObject->boundsUseTile().bottomLeft()));
        newPos.setX(16);
        setPos(newPos);
        setRotation(0);
        break;
    }
    case RTBMapObject::All:
        setPos(parentItem()->mapFromParent(QPointF(mMapObject->boundsUseTile().center())));
        setRotation(0);
        break;
    default:
        return;
    }

    const qreal arrowLength = 10;
    const qreal arrowHeadLength = 4.5;
    const qreal arrowHeadWidth = 7;
    const qreal bodyWidth = 3;

    // one arrow in the given direction
    if(direction != RTBMapObject::All)
    {
        QPainterPath path;
        path.moveTo(bodyWidth, 0);
        path.lineTo(0 + bodyWidth, arrowLength - arrowHeadLength);
        path.lineTo(arrowHeadWidth, arrowLength - arrowHeadLength);
        path.lineTo(0, arrowLength);
        path.lineTo(-arrowHeadWidth, arrowLength - arrowHeadLength);
        path.lineTo(0 - bodyWidth, arrowLength - arrowHeadLength);
        path.lineTo(0 - bodyWidth, 0);
        path.closeSubpath();
        path.translate(0, 3);

        painter->drawPath(path);
    }
    // everay direction one arrow
    else
    {
        QPainterPath path;
        path.moveTo(bodyWidth, 16);
        path.lineTo(0 + bodyWidth, arrowLength - arrowHeadLength + 16);
        path.lineTo(arrowHeadWidth, arrowLength - arrowHeadLength + 16);
        path.lineTo(0, arrowLength + 16);
        path.lineTo(-arrowHeadWidth, arrowLength - arrowHeadLength + 16);
        path.lineTo(0 - bodyWidth, arrowLength - arrowHeadLength + 16);
        path.lineTo(0 - bodyWidth, 16);
        path.closeSubpath();
        path.translate(0, 3);

        painter->drawPath(path);
        painter->rotate(90);
        painter->drawPath(path);
        painter->rotate(180);
        painter->drawPath(path);
        painter->rotate(-90);
        painter->drawPath(path);
    }
}


//=============================================================================

class RTBLaserBeamItem : public RTBVisualization
{
public:
    RTBLaserBeamItem(MapObject *mapObject, MapDocument *mapDocument, QGraphicsItem *parent = 0)
        : RTBVisualization(mapObject, mapDocument, parent)
        , mTargetCellX(0)
        , mTargetCellY(0)
        , mDeltaPoint(QPointF())
        , mDeltaStartPoint(QPointF())
        , mRotation(false)
        , mIsPaintingAllowed(true)
    {
        findTargetCell();
        updateBoundingRect();
        setDeltaPoint();

    }

    QRectF boundingRect() const { return mRect; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget);

    void findTargetCell();
    void updateBoundingRect();

    void setIsPaintingAllowed(bool isPaintingAllowed) { mIsPaintingAllowed = isPaintingAllowed; }
    bool isPaintingAllowed() { return mIsPaintingAllowed; }

private:
    void setDeltaPoint();
    bool checkCell(int cellX, int cellY, int type);
    bool onStartCell();

    void drawLaserBeam(QPainter *painter);
    void drawRotatingLaserBeam(QPainter *painter, int startDirection, int targetDirection);

    int mTargetCellX;
    int mTargetCellY;
    QPointF mDeltaPoint;
    QPointF mDeltaStartPoint;
    bool mRotation;

    bool mIsPaintingAllowed;

};

void RTBLaserBeamItem::setDeltaPoint()
{
    switch (mMapObject->cell().tile->id()) {
    case RTBMapObject::LaserBeamRight:
        mDeltaPoint = laserBeamRightDelta;
        mDeltaStartPoint = laserBeamRightStartDelta;
        break;
    case RTBMapObject::LaserBeamLeft:
        mDeltaPoint = laserBeamLeftDelta;
        mDeltaStartPoint = laserBeamLeftStartDelta;
        break;
    case RTBMapObject::LaserBeamTop:
        mDeltaPoint = laserBeamTopDelta;
        mDeltaStartPoint = laserBeamTopStartDelta;
        break;
    case RTBMapObject::LaserBeamBottom:
        mDeltaPoint = laserBeamBottomDelta;
        mDeltaStartPoint = laserBeamBottomStartDelta;
        break;
    default:
        break;
    }
}

void RTBLaserBeamItem::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *widget)
{
    // scale for the zoomlevel
    mScale = static_cast<MapView*>(widget->parent())->zoomable()->scale();

    painter->setRenderHint(QPainter::Antialiasing);
    QPen pen;

    RTBLaserBeam *obj = static_cast<RTBLaserBeam*>(mMapObject->rtbMapObject());
    if(obj->activatedOnStart())
        pen.setColor(Qt::red);
    else
        pen.setColor(Qt::black);


    if(obj->beamType() != RTBMapObject::BT1)
    {
        pen.setWidth(3);
        painter->setPen(pen);
        drawLaserBeam(painter);
    }
    else
    {
        pen.setWidth(2);
        painter->setPen(pen);
        drawRotatingLaserBeam(painter, obj->directionDegrees(), obj->targetDirectionDegrees());

    }
}

void RTBLaserBeamItem::drawLaserBeam(QPainter *painter)
{
    if(!mIsPaintingAllowed)
        return;

    findTargetCell();

    if(mRotation)
    {
        mRotation = false;
        updateBoundingRect();
    }

    // check if the blocking object is on the start cell, if so no laser beam needed
    if(!onStartCell())
    {
        QPointF start = mMapObject->boundsUseTile().center();
        QPointF target(mTargetCellX * 32, mTargetCellY * 32);

        QLineF laserLine((parentItem()->mapFromParent(start) + mDeltaStartPoint) * mScale, (parentItem()->mapFromParent(target) + mDeltaPoint) * mScale);
        painter->drawLine(laserLine);
    }

}

void RTBLaserBeamItem::drawRotatingLaserBeam(QPainter *painter, int startDirection, int targetDirection)
{
    if(!mRotation)
    {
        mRotation = true;
        updateBoundingRect();
    }

    QPointF start = mMapObject->boundsUseTile().center();
    start = (parentItem()->mapFromParent(start) + mDeltaStartPoint);
    QPointF target;
    int startDegree = 0;
    QRectF arcRect;

    QSizeF arcRectSizeHor(176, 160);
    QSizeF arcRectSizeVer(160, 176);

    // init parameter like start/target point, etc.
    switch (mMapObject->cell().tile->id()) {
    case RTBMapObject::LaserBeamRight:
    {
        target = QPointF(-64, -16);
        startDegree = 180;
        arcRect = QRectF(QPointF(-64, -96) * mScale, arcRectSizeHor * mScale);
        break;
    }
    case RTBMapObject::LaserBeamLeft:
    {
        target = QPointF(96, -16);
        startDegree = 0;
        arcRect = QRectF(QPointF(-80, -96) * mScale, arcRectSizeHor * mScale);
        break;
    }
    case RTBMapObject::LaserBeamTop:
    {
        target = QPointF(16, 64);
        startDegree = -90;
        arcRect = QRectF(QPointF(-64, -112) * mScale, arcRectSizeVer * mScale);
        break;
    }
    case RTBMapObject::LaserBeamBottom:
    {
        target = QPointF(16, -96);
        startDegree = 90;
        arcRect = QRectF(QPointF(-64, -96) * mScale, arcRectSizeVer * mScale);
        break;
    }
    default:
        break;
    }

    QPainterPath arcPath;
    QPainterPath path;

    // draw laser border and create the triangle path between the laser
    path.arcMoveTo(arcRect, (startDegree + targetDirection));
    QLineF laserLine(start * mScale, path.currentPosition());
    painter->drawLine(laserLine);
    path.lineTo(start * mScale);
    path.arcMoveTo(arcRect, (startDegree + startDirection));
    laserLine.setP2(path.currentPosition());
    painter->drawLine(laserLine);
    path.lineTo(start * mScale);
    path.arcMoveTo(arcRect, (startDegree + targetDirection));
    path.lineTo(laserLine.p2());

    // arc
    // set length of the arc
    int length = 0;
    if((0 < startDirection && 0 < targetDirection)
            || (0 > startDirection && 0 > targetDirection))
    {
        length = startDirection - targetDirection;
        length = abs(length);
    }
    else
        length = abs(startDirection) + abs(targetDirection);

    // draw arc
    arcPath.arcMoveTo(arcRect, (startDegree + startDirection));
    if(startDirection <= targetDirection)
    {
        arcPath.arcTo(arcRect, (startDegree + startDirection), length);
    }
    else
    {
        arcPath.arcTo(arcRect, (startDegree + targetDirection), length);
    }

    // connect arc with triangle
    arcPath.connectPath(path);
    painter->fillPath(arcPath, QBrush(painter->pen().color(), Qt::Dense6Pattern));
}

void RTBLaserBeamItem::findTargetCell()
{
    QPointF start = mMapObject->boundsUseTile().center();
    int cellX = start.x() / 32;
    int cellY = start.y() / 32;

    // direction of the laser beam
    switch (mMapObject->cell().tile->id()) {
    case RTBMapObject::LaserBeamRight:
        for(cellX++; cellX > 0 ; cellX--)
        {
            if(!checkCell(cellX - 1, cellY, RTBMapObject::LaserBeamRight))
                break;
        }
        break;
    case RTBMapObject::LaserBeamLeft:
        for(cellX--; cellX < mMapDocument->map()->width()-1; cellX++)
        {
            if(!checkCell(cellX + 1, cellY, RTBMapObject::LaserBeamLeft))
                break;
        }
        break;
    case RTBMapObject::LaserBeamTop:
        for(cellY--; cellY < mMapDocument->map()->height()-1; cellY++)
        {
            if(!checkCell(cellX, cellY + 1, RTBMapObject::LaserBeamTop))
                break;
        }
        break;
    case RTBMapObject::LaserBeamBottom:
        for(cellY++; cellY > 0 ; cellY--)
        {
            if(!checkCell(cellX, cellY - 1, RTBMapObject::LaserBeamBottom))
                break;
        }
        break;
    default:
        break;
    }

    if(mTargetCellX != cellX || mTargetCellY != cellY)
        update();

    mTargetCellX = cellX;
    mTargetCellY = cellY;
}

bool RTBLaserBeamItem::checkCell(int cellX, int cellY, int type)
{
    if(!mMapDocument)
        return false;

    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();
    Cell floorCell = floorLayer->cellAt(cellX, cellY);

    if(floorCell.isEmpty() && !mMapDocument->map()->rtbMap()->hasWall())
        return true;
    else if(floorCell.isEmpty() || floorCell.tile->id() == RTBMapSettings::WallBlock)
        return false;

    QList<MapObject *> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    for(MapObject * obj : objects)
    {
        // only projectile turret stop laser
        if(obj->rtbMapObject()->objectType() == RTBMapObject::ProjectileTurret)
        {
            QPointF position = obj->boundsUseTile().topLeft();
            qreal positionX;
            qreal positionY;
            // if the projectile turret is a half tile beside the laser, the laser will be not blocked
            // laser will be blocked, if projectile turret is in the line of the laser, also when the projectile turret is on a half cell position
            // depending on the laser orientation the cell value musst be rounded up or down
            switch (type) {
            case RTBMapObject::LaserBeamRight:
                positionX = floor(position.x() / 32);
                positionY = position.y() / 32;
                break;
            case RTBMapObject::LaserBeamLeft:
                positionX = round(position.x() / 32);
                positionY = position.y() / 32;
                break;
            case RTBMapObject::LaserBeamTop:
                positionX = position.x() / 32;
                positionY = round(position.y() / 32);
                break;
            case RTBMapObject::LaserBeamBottom:
                positionX = position.x() / 32;
                positionY = floor(position.y() / 32);
                break;
            default:
                break;
            }

            if(positionX == cellX && positionY == cellY)
                return false;
        }
    }

    // cell is not empty an nothing in this cell stop the laser
    return true;
}

void RTBLaserBeamItem::updateBoundingRect()
{
    switch (mMapObject->cell().tile->id()) {
    case RTBMapObject::LaserBeamRight:
    {
        if(mRotation)
        {
            mRect = QRectF(-64, -96, 96, 192);
        }
        else
        {
            QPointF start = mMapObject->boundsUseTile().topLeft();
            QPointF topLeft(-start.x(), -32);
            mRect = QRectF(topLeft, QPointF(32, 0));
        }
        break;
    }
    case RTBMapObject::LaserBeamLeft:
    {
        if(mRotation)
        {
            mRect = QRectF(0, -96, 96, 192);
        }
        else
        {
            QSize size = mMapDocument->map()->size();
            QPointF bottomRight = QPointF(size.width() * 32, 0);
            mRect = QRectF(QPointF(0, -32), bottomRight);
        }
        break;
    }
    case RTBMapObject::LaserBeamTop:
    {
        if(mRotation)
        {
            mRect = QRectF(-64, -32, 192, 96);
        }
        else
        {
            QSize size = mMapDocument->map()->size();
            QPointF bottomRight = QPointF(32, size.height() * 32);
            mRect = QRectF(QPointF(0, -32), bottomRight);
        }
        break;
    }
    case RTBMapObject::LaserBeamBottom:
    {
        if(mRotation)
        {
            mRect = QRectF(-64, -96, 192, 96);
        }
        else
        {
            QPointF start = mMapObject->boundsUseTile().topLeft();
            QPointF topLeft(0, -start.y() - 32);
            mRect = QRectF(topLeft, QPointF(32, 0));
        }
        break;
    }
    default:
        mRect = QRectF();
        break;
    }

    update();
}

bool RTBLaserBeamItem::onStartCell()
{
    QPointF start = mMapObject->boundsUseTile().center();

    switch (mMapObject->cell().tile->id()) {
    case RTBMapObject::LaserBeamRight:
        if(mTargetCellX-1 == int(start.x() / 32))
            return true;
        break;
    case RTBMapObject::LaserBeamLeft:
        if(mTargetCellX+1 == int(start.x() / 32))
            return true;
        break;
    case RTBMapObject::LaserBeamTop:
        if(mTargetCellY+1 == int(start.y() / 32))
            return true;
        break;
    case RTBMapObject::LaserBeamBottom:
        if(mTargetCellY-1 == int(start.y() / 32))
            return true;
        break;
    default:
        break;
    }

    return false;
}

//=============================================================================

RTBMapObjectItem::RTBMapObjectItem(MapObject *mapObject, MapDocument *mapDocument, QGraphicsItem *parent)
    : mMapObject(mapObject)
    , mMapDocument(mapDocument)
    , mRTBVisualization(new RTBVisualization(mapObject, mapDocument, parent))
    , mRTBMapObjectValidate(new RTBMapObjectValidate(mapObject, mapDocument, parent))
    , mRTBMapObjectLabel(0)
    , mRTBLaserBeamItem(0)
    , mVisualizePropHandle(0)
{ 
    switch (mMapObject->rtbMapObject()->objectType()) {
    case RTBMapObject::LaserBeam:
        mRTBLaserBeamItem = new RTBLaserBeamItem(mapObject, mapDocument, parent);
        mRTBMapObjectLabel = new RTBMapObjectLabel(mapObject, mapDocument, parent);
        break;
    case RTBMapObject::FloorText:
    case RTBMapObject::Teleporter:
    case RTBMapObject::CameraTrigger:
        mVisualizePropHandle = new RTBVisualizePropHandle(mapObject, mapDocument, parent);
        break;
    case RTBMapObject::Button:
    case RTBMapObject::ProjectileTurret:
        mVisualizePropHandle = new RTBVisualizePropHandle(mapObject, mapDocument, parent);
        mRTBMapObjectLabel = new RTBMapObjectLabel(mapObject, mapDocument, parent);
        break;
    case RTBMapObject::CustomFloorTrap:
    case RTBMapObject::MovingFloorTrapSpawner:
    case RTBMapObject::NPCBallSpawner:
        mRTBMapObjectLabel = new RTBMapObjectLabel(mapObject, mapDocument, parent);
        break;
    case RTBMapObject::Orb:
        break;
    default:
        break;
    }
}

void RTBMapObjectItem::updateBoundingRect()
{
    if(mRTBLaserBeamItem)
        static_cast<RTBLaserBeamItem*>(mRTBLaserBeamItem)->updateBoundingRect();
}

void RTBMapObjectItem::setIsPaintingAllowed(bool isPaintingAllowed)
{
    if(mIsPaintingAllowed == isPaintingAllowed)
        return;

    mIsPaintingAllowed = isPaintingAllowed;

    if(mRTBLaserBeamItem)
        static_cast<RTBLaserBeamItem*>(mRTBLaserBeamItem)->setIsPaintingAllowed(isPaintingAllowed);
}
