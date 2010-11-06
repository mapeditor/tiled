/*
 * stampbrush.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010 Stefan Beller <stefanbeller@googlemail.com>
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

#include "stampbrush.h"

#include "brushitem.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "tilelayer.h"

#include <math.h>
#include <QVector>

using namespace Tiled;
using namespace Tiled::Internal;

StampBrush::StampBrush(QObject *parent)
    : AbstractTileTool(tr("Stamp Brush"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-clone.png")),
                       QKeySequence(tr("B")),
                       parent)
    , mStamp(0)
    , mStampX(0), mStampY(0)
    , mBrushBehavior(Free)
    , mLastStampX(0)
    , mLastStampY(0)
{
}

StampBrush::~StampBrush()
{
    delete mStamp;
}

/**
 * Returns a lists of points on a circle.
 * (x0,y0) is the midpoint
 * (x1,y1) to determines the radius.
 * It is adapted from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 */
static QVector<QPoint> rasterCircle(int x0, int y0, int x1, int y1)
{
    QVector<QPoint> ret;

    const int radius = sqrt((x0 - x1) * (x0 - x1) +
                            (y0 - y1) * (y0 - y1));

    int f = 1 - radius;
    int ddF_x = 1;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;

    ret += QPoint(x0, y0 + radius);
    ret += QPoint(x0, y0 - radius);
    ret += QPoint(x0 + radius, y0);
    ret += QPoint(x0 - radius, y0);

    while (x < y) {
        // ddF_x == 2 * x + 1;
        // ddF_y == -2 * y;
        // f == x*x + y*y - radius*radius + 2*x - y + 1;
        if (f >= 0) {
          y--;
          ddF_y += 2;
          f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        ret += QPoint(x0 + x, y0 + y);
        ret += QPoint(x0 - x, y0 + y);
        ret += QPoint(x0 + x, y0 - y);
        ret += QPoint(x0 - x, y0 - y);
        ret += QPoint(x0 + y, y0 + x);
        ret += QPoint(x0 - y, y0 + x);
        ret += QPoint(x0 + y, y0 - x);
        ret += QPoint(x0 - y, y0 - x);
    }

    return ret;
}

/**
 * Returns the lists of points on a line from (x0,y0) to (x1,y1).
 *
 * This is an implementation of bresenhams line algorithm, initially copied
 * from http://en.wikipedia.org/wiki/Bresenham's_line_algorithm#Optimization
 * changed to C++ syntax.
 */
static QVector<QPoint> calculateLine(int x0, int y0, int x1, int y1)
{
    QVector<QPoint> ret;

    bool steep = qAbs(y1 - y0) > qAbs(x1 - x0);
    if (steep) {
        qSwap(x0, y0);
        qSwap(x1, y1);
    }
    if (x0 > x1) {
        qSwap(x0, x1);
        qSwap(y0, y1);
    }
    const int deltax = x1 - x0;
    const int deltay = qAbs(y1 - y0);
    int error = deltax / 2;
    int ystep;
    int y = y0;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    for (int x = x0; x < x1 + 1 ; x++) {
        if (steep)
            ret += QPoint(y, x);
        else
            ret += QPoint(x, y);
        error = error - deltay;
        if (error < 0) {
             y = y + ystep;
             error = error + deltax;
        }
    }

    return ret;
}

void StampBrush::tilePositionChanged(const QPoint &)
{
    updatePosition();
    switch (mBrushBehavior) {
    case Paint:
        doPaint(true, mStampX, mStampY);
        break;
    case Line:
        configureBrush(calculateLine(mLastStampX, mLastStampY,
                                     mStampX, mStampY));
        break;
    case Circle:
        configureBrush(rasterCircle(mLastStampX, mLastStampY,
                                    mStampX, mStampY));
        break;
    case Capture:
        brushItem()->setTileRegion(capturedArea());
        break;
    default:
    case Free:
        // do nothing here
        break;
    }
}

void StampBrush::mousePressed(const QPointF &, Qt::MouseButton button,
                              Qt::KeyboardModifiers)
{
    if (!brushItem()->isVisible())
        return;

    if (button == Qt::LeftButton) {
        if (mBrushBehavior == Line || mBrushBehavior == Circle) {
            if (mBrushBehavior == Line) {
                configureBrush(calculateLine(mLastStampX, mLastStampY,
                                             mStampX, mStampY));
            } else if (mBrushBehavior == Circle) {
                configureBrush(rasterCircle(mLastStampX, mLastStampY,
                                            mStampX, mStampY));
            }
            doPaint(false, 0, 0);
            mLastStampX = mStampX;
            mLastStampY = mStampY;
        } else {
            beginPaint();
        }
    } else {
        if (button == Qt::RightButton)
            beginCapture();
    }
}

void StampBrush::mouseReleased(const QPointF &, Qt::MouseButton button)
{
    switch (mBrushBehavior) {
    case Capture:
        if (button == Qt::RightButton) {
            endCapture();
            mBrushBehavior = Free;
        }
        break;
    case Paint:
        if (button == Qt::LeftButton)
            mBrushBehavior = Free;
    default:
        // do nothing?
        break;
    }
}

void StampBrush::configureBrush(const QVector<QPoint> &list)
{
    if (!mStamp)
        return;

    QRegion reg;
    QRegion stampRegion(mStamp->region());

    Map *map = mapDocument()->map();

    TileLayer *stamp = new TileLayer(QString(), 0, 0,
                                     map->width(), map->height());

    foreach (QPoint p, list) {
        const QRegion update = stampRegion.translated(p.x() - mStampX,
                                                      p.y() - mStampY);
        if (!reg.intersects(update)) {
            reg += update;
            stamp->merge(p, mStamp);
        }
    }
    brushItem()->setTileLayer(stamp);
}

void StampBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (!mStamp)
        return;

    if (modifiers & Qt::ShiftModifier)
        mBrushBehavior = Line;
    else if (modifiers & Qt::ControlModifier)
        mBrushBehavior = Circle;
    else
        mBrushBehavior = Free;

    brushItem()->setTileLayer(mStamp->clone()->asTileLayer());
    updatePosition();
}

void StampBrush::languageChanged()
{
    setName(tr("Stamp Brush"));
    setShortcut(QKeySequence(tr("B")));
}

void StampBrush::mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    // Reset the brush, since it probably became invalid
    brushItem()->setTileRegion(QRegion());
    setStamp(0);
}

void StampBrush::setStamp(TileLayer *stamp)
{
    if (mStamp == stamp)
        return;

    brushItem()->setTileLayer(stamp);
    delete mStamp;
    mStamp = stamp;

    updatePosition();
}

void StampBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint(false, mStampX, mStampY);
}

void StampBrush::beginCapture()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Capture;

    mCaptureStart = tilePosition();

    setStamp(0);
}

void StampBrush::endCapture()
{
    if (mBrushBehavior != Capture)
        return;

    mBrushBehavior = Free;

    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    // Intersect with the layer and translate to layer coordinates
    QRect captured = capturedArea();
    captured.intersect(QRect(tileLayer->x(), tileLayer->y(),
                             tileLayer->width(), tileLayer->height()));

    if (captured.isValid()) {
        captured.translate(-tileLayer->x(), -tileLayer->y());
        TileLayer *capture = tileLayer->copy(captured);
        emit currentTilesChanged(capture);
        // A copy will have been created, so delete this version
        delete capture;
    } else {
        updatePosition();
    }
}

QRect StampBrush::capturedArea() const
{
    QRect captured = QRect(mCaptureStart, tilePosition()).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

void StampBrush::doPaint(bool mergeable, int whereX, int whereY)
{
    if (!mStamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->bounds().intersects(QRect(whereX, whereY,
                                              mStamp->width(),
                                              mStamp->height())))
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(), tileLayer,
                            whereX, whereY, brushItem()->tileLayer());
    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    mLastStampX = mStampX;
    mLastStampY = mStampY;
}

/**
 * Updates the position of the brush item.
 */
void StampBrush::updatePosition()
{
    const QPoint tilePos = tilePosition();

    if (mStamp) {
        mStampX = tilePos.x() - mStamp->width() / 2;
        mStampY = tilePos.y() - mStamp->height() / 2;
        brushItem()->setTileLayerPosition(QPoint(mStampX, mStampY));
    } else {
        brushItem()->setTileRegion(QRect(tilePos, QSize(1, 1)));
    }
}
