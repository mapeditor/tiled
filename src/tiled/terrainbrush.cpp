/*
 * terrainbrush.cpp
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

#include "terrainbrush.h"

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

TerrainBrush::TerrainBrush(QObject *parent)
    : AbstractTileTool(tr("Terrain Brush"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-clone.png")),
                       QKeySequence(tr("T")),
                       parent)
    , mTerrain(NULL)
    , mPaintX(0), mPaintY(0)
    , mBrushBehavior(Free)
    , mStampReferenceX(0)
    , mStampReferenceY(0)
{
}

TerrainBrush::~TerrainBrush()
{
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

void TerrainBrush::tilePositionChanged(const QPoint &)
{
    const int x = mPaintX;
    const int y = mPaintY;
    updatePosition();
    switch (mBrushBehavior) {
    case Paint:
        foreach (const QPoint &p, calculateLine(x, y, mPaintX, mPaintY))
            doPaint(true, p.x(), p.y());
        break;
    case LineStartSet:
        configureBrush(calculateLine(mStampReferenceX, mStampReferenceY,
                                     mPaintX, mPaintY));
        break;
    case Capture:
        brushItem()->setTileRegion(capturedArea());
        break;
    case Line:
    case Free:
        updatePosition();
        break;
    }
}

void TerrainBrush::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (!brushItem()->isVisible())
        return;

    if (event->button() == Qt::LeftButton) {
        switch (mBrushBehavior) {
        case Line:
            mStampReferenceX = mPaintX;
            mStampReferenceY = mPaintY;
            mBrushBehavior = LineStartSet;
            break;
        case LineStartSet:
            doPaint(false, 0, 0);
            mStampReferenceX = mPaintX;
            mStampReferenceY = mPaintY;
            break;
        case Paint:
            beginPaint();
            break;
        case Free:
            beginPaint();
            mBrushBehavior = Paint;
            break;
        case Capture:
            break;
        }
    } else {
        if (event->button() == Qt::RightButton)
            beginCapture();
    }
}

void TerrainBrush::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mBrushBehavior) {
    case Capture:
        if (event->button() == Qt::RightButton) {
            endCapture();
            mBrushBehavior = Free;
        }
        break;
    case Paint:
        if (event->button() == Qt::LeftButton)
            mBrushBehavior = Free;
    default:
        // do nothing?
        break;
    }
}

void TerrainBrush::configureBrush(const QVector<QPoint> &list)
{
    if (!mTerrain)
        return;
/*
    QRegion reg;
    QRegion stampRegion;

    stampRegion = mStamp->region();

    Map *map = mapDocument()->map();

    TileLayer *stamp = new TileLayer(QString(), 0, 0,
                                     map->width(), map->height());

    foreach (const QPoint p, list) {
        const QRegion update = stampRegion.translated(p.x() - mStampX,
                                                      p.y() - mStampY);
        if (!reg.intersects(update)) {
            reg += update;

            if (mIsRandom) {
                TileLayer *newStamp = getRandomTileLayer();
                stamp->merge(p, newStamp);
                delete newStamp;
            } else {
                stamp->merge(p, mStamp);
            }

        }
    }

    brushItem()->setTileLayer(stamp);
    delete stamp;
*/
}

void TerrainBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (!mTerrain)
        return;

    if (modifiers & Qt::ShiftModifier) {
        mBrushBehavior = Line;
    } else {
        mBrushBehavior = Free;
    }

/*
    brushItem()->setTileLayer(mTerrain);
*/
    updatePosition();
}

void TerrainBrush::languageChanged()
{
    setName(tr("Terain Brush"));
    setShortcut(QKeySequence(tr("T")));
}

void TerrainBrush::mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    // Reset the brush, since it probably became invalid
    brushItem()->setTileRegion(QRegion());
    setTerrain(NULL);
}

void TerrainBrush::setTerrain(TerrainType *terrain)
{
    if (mTerrain == terrain)
        return;

    mTerrain = terrain;
/*
    brushItem()->setTileLayer(mStamp);
*/
    updatePosition();
}

void TerrainBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint(false, mPaintX, mPaintY);
}

void TerrainBrush::beginCapture()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Capture;

    mCaptureStart = tilePosition();

    setTerrain(NULL);
}

void TerrainBrush::endCapture()
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

QRect TerrainBrush::capturedArea() const
{
    QRect captured = QRect(mCaptureStart, tilePosition()).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

void TerrainBrush::doPaint(bool mergeable, int whereX, int whereY)
{
    TileLayer *stamp = brushItem()->tileLayer();

    if (!stamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->bounds().intersects(QRect(whereX, whereY,
                                              stamp->width(),
                                              stamp->height())))
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(), tileLayer,
                            whereX, whereY, stamp);
    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    mapDocument()->emitRegionEdited(brushItem()->tileRegion(), tileLayer);
}

/**
 * Updates the position of the brush item.
 */
void TerrainBrush::updatePosition()
{
    const QPoint tilePos = tilePosition();

    if (!brushItem()->tileLayer()) {
        brushItem()->setTileRegion(QRect(tilePos, QSize(1, 1)));
        mPaintX = tilePos.x();
        mPaintY = tilePos.y();
    }

    mPaintX = tilePos.x();
    mPaintY = tilePos.y();
    brushItem()->setTileLayerPosition(QPoint(mPaintX, mPaintY));
}
