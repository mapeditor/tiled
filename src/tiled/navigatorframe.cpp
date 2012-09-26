/*
 * navigatorframe.cpp
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "navigatorframe.h"

#include "documentmanager.h"
#include "imagelayer.h"
#include "mapdocument.h"
#include "map.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapview.h"
#include "objectgroup.h"
#include "preferences.h"
#include "tilelayer.h"
#include "zoomable.h"

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>

using namespace Tiled;
using namespace Tiled::Internal;

NavigatorFrame::NavigatorFrame(QWidget *parent)
    : QFrame(parent)
    , mMapDocument(0)
    , mHScrollBar(0)
    , mVScrollBar(0)
    , mDragging(false)
    , mMouseMoveCursorState(false)
{
    setFrameStyle(QFrame::NoFrame);
    mRenderFlags = NavigatorRenderFlags(DrawTiles | DrawObjects | DrawImages | IgnoreInvisibleLayer);

    // for cursor changes
    this->setMouseTracking(true);
}

void NavigatorFrame::setMapDocument(MapDocument *map)
{
    if (mHScrollBar)
        disconnect(mHScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollbarChanged()));
    if (mVScrollBar)
        disconnect(mVScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollbarChanged()));

    MapView *mapView = DocumentManager::instance()->currentMapView();
    if (mapView) {
        mHScrollBar = mapView->horizontalScrollBar();
        mVScrollBar = mapView->verticalScrollBar();

        connect(mHScrollBar, SIGNAL(valueChanged(int)), SLOT(scrollbarChanged()));
        connect(mVScrollBar, SIGNAL(valueChanged(int)), SLOT(scrollbarChanged()));
    }

    mMapDocument = map;
    redrawMapAndFrame();
}

void NavigatorFrame::redrawFrame()
{
    update();
}

void NavigatorFrame::redrawMapAndFrame()
{
    recreateMapImage();
    update();
}

NavigatorFrame::NavigatorRenderFlags NavigatorFrame::renderFlags() const
{
    return mRenderFlags;
}

void NavigatorFrame::setRenderFlags(NavigatorFrame::NavigatorRenderFlags flags)
{
    mRenderFlags = flags;
}

void NavigatorFrame::paintEvent(QPaintEvent *pe)
{
    QFrame::paintEvent(pe);
    QPainter p(this);

    QPoint frameCenter = QPoint(size().width(), size().height()) * 0.5;
    frameCenter -= QPoint(imageContentRect.size().width() + 2,
                          imageContentRect.size().height() + 2) * 0.5;
    p.translate(frameCenter);

    // outline
    p.setBrush(QBrush(QColor(255, 255, 255)));
    p.setPen(Qt::NoPen);
    p.drawRect(imageContentRect.x(),
               imageContentRect.y(),
               imageContentRect.width() + 1,
               imageContentRect.height() + 1);

    if (!mMapImage.isNull())
        p.drawImage(1, 1, mMapImage);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(0, 0, 0)));
    p.drawRect(imageContentRect.x(),
               imageContentRect.y(),
               imageContentRect.width() + 1,
               imageContentRect.height() + 1);

    MapView *mapView = DocumentManager::instance()->currentMapView();
    if (mapView) {
        QRectF drawRect = viewportRect();
        p.setPen(QColor(0,0,0));
        p.translate(0, 1.5);
        p.drawRect(drawRect);

        p.translate(0, -1.5);
        p.setPen(QPen(QBrush(QColor(255, 0, 0)), 2));
        p.drawRect(drawRect);
    }
}

void NavigatorFrame::resizeEvent(QResizeEvent *)
{
    recreateMapImage();
}

void NavigatorFrame::recreateMapImage()
{
    QSize newImageSize = size();
    if (size().width() < 50 ||
        size().height() < 50)
        newImageSize = QSize(50, 50);

    if (mMapImage.isNull())
        mMapImage = QImage(newImageSize, QImage::Format_ARGB32);
    else if (mMapImage.size().width() < newImageSize.width() ||
             mMapImage.size().height() < newImageSize.height())
        resizeImage(newImageSize);

    if (mMapDocument)
        renderMapToImage();
}

void NavigatorFrame::renderMapToImage()
{
    if (size().width() < 2 ||
        size().height() < 2 ||
        !isVisible())
        return;

    MapRenderer *renderer = mMapDocument->renderer();

    bool drawObjects = mRenderFlags.testFlag(DrawObjects);
    bool drawTiles = mRenderFlags.testFlag(DrawTiles);
    bool drawImages = mRenderFlags.testFlag(DrawImages);
    bool drawTileGrid = mRenderFlags.testFlag(DrawGrid);
    bool visibleLayersOnly = mRenderFlags.testFlag(IgnoreInvisibleLayer);

    // Remember the current render flags
    const Tiled::RenderFlags renderFlags = renderer->flags();
    renderer->setFlag(ShowTileObjectOutlines, false);

    mMapImage.fill(Qt::transparent);
    QPainter painter(&mMapImage);
    QTransform trans;
    QSizeF mapSize = renderer->mapSize();
    QSizeF frameSize = size() - QSizeF(2, 2);
    float xScale = frameSize.width() / mapSize.width();
    float yScale = frameSize.height() / mapSize.height();
    int tx = mapSize.width() * xScale;
    int ty = mapSize.height() * xScale;
    if (tx <= frameSize.width() &&
        ty <= frameSize.height()) {
        imageContentRect.setSize(QSize(tx,ty));
        trans.scale(xScale, xScale);
    }
    else {
        imageContentRect.setSize(QSize(mapSize.width() * yScale, mapSize.height() * yScale));
        trans.scale(yScale, yScale);
    }
    painter.setTransform(trans);

    foreach (const Layer *layer, mMapDocument->map()->layers()) {
        if (visibleLayersOnly && !layer->isVisible())
            continue;

        painter.setOpacity(layer->opacity());

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ObjectGroup *objGroup = dynamic_cast<const ObjectGroup*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer && drawTiles) {
            renderer->drawTileLayer(&painter, tileLayer);
        } else if (objGroup && drawObjects) {
            foreach (const MapObject *object, objGroup->objects()) {
                const QColor color = MapObjectItem::objectColor(object);
                renderer->drawMapObject(&painter, object, color);
            }
        } else if (imageLayer && drawImages) {
            renderer->drawImageLayer(&painter, imageLayer);
        }
    }

    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        renderer->drawGrid(&painter, QRectF(QPointF(), renderer->mapSize()),
                           prefs->gridColor());
    }

    renderer->setFlags(renderFlags);
}

void NavigatorFrame::resizeImage(const QSize &newSize)
{
    if (mMapImage.size() == newSize)
        return;

    mMapImage = QImage(newSize, QImage::Format_ARGB32);
}

void NavigatorFrame::centerViewOnLocalPixel(const QPointF &centerPos, int delta)
{
    MapView *mapView = DocumentManager::instance()->currentMapView();
    if (!mapView)
        return;

    // viewRect => the size of the entire scene in pixel
    QRectF mapContentRect = mapView->sceneRect();
    mapView->setTransformationAnchor(QGraphicsView::NoAnchor);
    if (delta != 0)
        mapView->zoomable()->handleWheelDelta(delta);
    QPointF viewCenterPos(
                centerPos.x() / imageContentRect.width() * mapContentRect.width(),
                centerPos.y() / imageContentRect.height() * mapContentRect.height());
    mapView->centerOn(viewCenterPos);
    mapView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

void NavigatorFrame::scrollbarChanged()
{
    update();
}

void NavigatorFrame::wheelEvent(QWheelEvent *event)
{    
    if (event->orientation() == Qt::Vertical) {
        // cursor position on map
        QPoint frameCenter = QPoint(size().width(), size().height()) * 0.5;
        frameCenter -= QPoint(imageContentRect.size().width() + 2,
                              imageContentRect.size().height() + 2) * 0.5;
        QPointF cursorPos = event->pos() - frameCenter;

        centerViewOnLocalPixel(cursorPos, event->delta());
        update();
        return;
    }

    QFrame::wheelEvent(event);
}

void NavigatorFrame::mousePressEvent(QMouseEvent *event)
{       
    if (event->button() == Qt::LeftButton) {
        // cursor position on map
        QPoint frameCenter = QPoint(size().width(), size().height()) * 0.5;
        frameCenter -= QPoint(imageContentRect.size().width() + 2,
                              imageContentRect.size().height() + 2) * 0.5;
        QPointF cursorPos = event->pos() - frameCenter;

        if (cursorPos.x() >=0 &&
            cursorPos.y() >=0 &&
            cursorPos.x() < imageContentRect.width() &&
            cursorPos.y() < imageContentRect.height()) {

            QRectF viewPort = viewportRect();
            if (viewPort.contains(cursorPos)) {
                mDragOffset = viewPort.center() - cursorPos;
                cursorPos += mDragOffset;
                centerViewOnLocalPixel(cursorPos);
                mDragging = true;

                QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
            }
            else {
                centerViewOnLocalPixel(cursorPos);
            }
        }
    }
}

void NavigatorFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mDragging) {
            mDragging = false;
            QApplication::restoreOverrideCursor();
        }
    }
}

void NavigatorFrame::mouseMoveEvent(QMouseEvent *event)
{    
    // cursor position on map
    QPoint frameCenter = QPoint(size().width(), size().height()) * 0.5;
    frameCenter -= QPoint(imageContentRect.size().width() + 2,
                          imageContentRect.size().height() + 2) * 0.5;
    QPointF cursorPos = event->pos() - frameCenter;

    if (mDragging) {
        cursorPos += mDragOffset;
        centerViewOnLocalPixel(cursorPos);
    }
    else {
        if (cursorPos.x() >=0 &&
            cursorPos.y() >=0 &&
            cursorPos.x() < imageContentRect.width() &&
            cursorPos.y() < imageContentRect.height()) {

            QRectF viewPort = viewportRect();
            if (viewPort.contains(cursorPos)) {
                if (!mMouseMoveCursorState) {
                    QApplication::setOverrideCursor(QCursor(Qt::OpenHandCursor));
                    mMouseMoveCursorState = true;
                }
            }
            else {
                if (mMouseMoveCursorState) {
                    QApplication::restoreOverrideCursor();
                    mMouseMoveCursorState = false;
                }
            }
        }
    }
}

void NavigatorFrame::leaveEvent(QEvent *e)
{
    QFrame::leaveEvent(e);
    if (mMouseMoveCursorState) {
        QApplication::restoreOverrideCursor();
        mMouseMoveCursorState = false;
    }
}

QRectF NavigatorFrame::viewportRect() const
{
    MapView *mapView = DocumentManager::instance()->currentMapView();
    if (!mapView)
        return QRectF(0, 0, 1, 1);

    // viewRect => the size of the entire scene in pixel
    QRectF mapContentRect = mapView->sceneRect();
    QRectF visibleRect = mapView->mapToScene(mapView->viewport()->geometry()).boundingRect();
    return QRectF(visibleRect.x() / mapContentRect.width() * imageContentRect.width(),
                  visibleRect.y() / mapContentRect.height() * imageContentRect.height(),
                  visibleRect.width() / mapContentRect.width() * imageContentRect.width(),
                  visibleRect.height() / mapContentRect.height() * imageContentRect.height());
}
