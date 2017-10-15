/*
 * terrainbrush.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2012, Manu Evans <turkeyman@gmail.com>
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

#include "addremovetileset.h"
#include "brushitem.h"
#include "containerhelpers.h"
#include "geometry.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "randompicker.h"
#include "staggeredrenderer.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QVector>

#include <climits>

using namespace Tiled;
using namespace Tiled::Internal;

TerrainBrush::TerrainBrush(QObject *parent)
    : AbstractTileTool(tr("Terrain Brush"),
                       QIcon(QLatin1String(
                               ":images/24x24/terrain-edit.png")),
                       QKeySequence(tr("T")),
                       nullptr,
                       parent)
    , mTerrain(nullptr)
    , mPaintX(0), mPaintY(0)
    , mIsActive(false)
    , mBrushBehavior(Free)
    , mMirrorDiagonally(false)
    , mLineReferenceX(0)
    , mLineReferenceY(0)
{
    setBrushMode(PaintTile);
}

TerrainBrush::~TerrainBrush()
{
}

void TerrainBrush::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);
    mIsActive = true;
}

void TerrainBrush::deactivate(MapScene *scene)
{
    AbstractTileTool::deactivate(scene);
    mIsActive = false;
}

void TerrainBrush::tilePositionChanged(const QPoint &pos)
{
    switch (mBrushBehavior) {
    case Paint: {
        int x = mPaintX;
        int y = mPaintY;
        foreach (const QPoint &p, pointsOnLine(x, y, pos.x(), pos.y())) {
            updateBrush(p);
            doPaint(true);
        }
        break;
    }
    case LineStartSet: {
        QVector<QPoint> lineList = pointsOnLine(mLineReferenceX, mLineReferenceY,
                                                pos.x(), pos.y());
        updateBrush(pos, &lineList);
        break;
    }
    case Line:
    case Free:
        updateBrush(pos);
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
            mLineReferenceX = mPaintX;
            mLineReferenceY = mPaintY;
            mBrushBehavior = LineStartSet;
            break;
        case LineStartSet:
            doPaint(false);
            mLineReferenceX = mPaintX;
            mLineReferenceY = mPaintY;
            break;
        case Paint:
            beginPaint();
            break;
        case Free:
            beginPaint();
            mBrushBehavior = Paint;
            break;
        }
    } else {
        if (event->button() == Qt::RightButton)
            capture();
    }
}

void TerrainBrush::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mBrushBehavior) {
    case Paint:
        if (event->button() == Qt::LeftButton)
            mBrushBehavior = Free;
        break;
    default:
        // do nothing?
        break;
    }
}

void TerrainBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    const bool lineMode = modifiers & Qt::ShiftModifier;

    if (lineMode != (mBrushBehavior == Line ||
                     mBrushBehavior == LineStartSet)) {
        mBrushBehavior = lineMode ? Line : Free;
    }

    mMirrorDiagonally = modifiers & Qt::AltModifier;

    setBrushMode((modifiers & Qt::ControlModifier) ? PaintVertex : PaintTile);
    updateBrush(tilePosition());
}

void TerrainBrush::languageChanged()
{
    setName(tr("Terrain Brush"));
    setShortcut(QKeySequence(tr("T")));
}

void TerrainBrush::mapDocumentChanged(MapDocument *oldDocument,
                                      MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    // Reset the brush, since it probably became invalid
    brushItem()->clear();
}

void TerrainBrush::setTerrain(const Terrain *terrain)
{
    if (mTerrain == terrain)
        return;

    mTerrain = terrain;

    if (mIsActive && brushItem()->isVisible())
        updateBrush(tilePosition());
}

void TerrainBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint(false);
}

void TerrainBrush::capture()
{
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    // TODO: we need to know which corner the mouse is closest to...

    const QPoint position = tilePosition() - tileLayer->position();

    Terrain *terrain = nullptr;

    const Cell &cell = tileLayer->cellAt(position);
    if (const Tile *tile = cell.tile())
        terrain = tile->terrainAtCorner(0);

    setTerrain(terrain);
    emit terrainCaptured(terrain);
}

void TerrainBrush::doPaint(bool mergeable)
{
    TileLayer *stamp = brushItem()->tileLayer().data();

    if (!stamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->isUnlocked())
        return;

    if (!tileLayer->rect().intersects(stamp->bounds()) && !mapDocument()->map()->infinite())
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(), tileLayer,
                                               stamp->x(), stamp->y(),
                                               stamp, brushItem()->tileRegion());

    if (mTerrain && !contains(mapDocument()->map()->tilesets(), mTerrain->tileset()))
        new AddTileset(mapDocument(), mTerrain->tileset()->sharedPointer(), paint);

    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    emit mapDocument()->regionEdited(brushItem()->tileRegion(), tileLayer);
}

static Tile *findBestTile(const Tileset &tileset, unsigned terrain, unsigned considerationMask)
{
    // we should have hooked 0xFFFFFFFF terrains outside this function
    Q_ASSERT(terrain != 0xFFFFFFFF);

    RandomPicker<Tile*, float> matches;
    int penalty = INT_MAX;

    // TODO: this is a slow linear search, perhaps we could use a better find algorithm...
    for (Tile *t : tileset.tiles()) {
        if ((t->terrain() & considerationMask) != (terrain & considerationMask))
            continue;

        // calculate the tile transition penalty based on shortest distance to target terrain type
        int tr = tileset.terrainTransitionPenalty(t->terrain() >> 24, terrain >> 24);
        int tl = tileset.terrainTransitionPenalty((t->terrain() >> 16) & 0xFF, (terrain >> 16) & 0xFF);
        int br = tileset.terrainTransitionPenalty((t->terrain() >> 8) & 0xFF, (terrain >> 8) & 0xFF);
        int bl = tileset.terrainTransitionPenalty(t->terrain() & 0xFF, terrain & 0xFF);

        // if there is no path to the destination terrain, this isn't a useful transition
        if (tr < 0 || tl < 0 || br < 0 || bl < 0)
            continue;

        // add tile to the candidate list
        int transitionPenalty = tr + tl + br + bl;
        if (transitionPenalty <= penalty) {
            if (transitionPenalty < penalty)
                matches.clear();
            penalty = transitionPenalty;

            matches.add(t, t->probability());
        }
    }

    // choose a candidate at random, with consideration for probability
    if (!matches.isEmpty())
        return matches.pick();

    // TODO: conveniently, the null tile doesn't currently work, but when it does, we need to signal a failure to find any matches some other way
    return nullptr;
}

static unsigned terrain(const Tile *tile)
{
    return tile ? tile->terrain() : 0xFFFFFFFF;
}

static unsigned short topEdge(const Tile *tile)
{
    return terrain(tile) >> 16;
}

static unsigned short bottomEdge(const Tile *tile)
{
    return terrain(tile) & 0xFFFF;
}

static unsigned short leftEdge(const Tile *tile)
{
    unsigned t = terrain(tile);
    return((t >> 16) & 0xFF00) | ((t >> 8) & 0xFF);
}

static unsigned short rightEdge(const Tile *tile)
{
    unsigned t = terrain(tile);
    return ((t >> 8) & 0xFF00) | (t & 0xFF);
}

static bool contains(const QRect &bounds, QPoint point)
{
    return bounds.left() <= point.x() &&
           bounds.top() <= point.y() &&
           bounds.right() >= point.x() &&
           bounds.bottom() >= point.y();
}

static bool checked(const TileLayer &tileLayer, QPoint point)
{
    return tileLayer.cellAt(point).checked();
}

namespace {

struct ConsiderationPoint : public QPoint
{
    ConsiderationPoint()
        : paintCorner(0)
    {}

    ConsiderationPoint(QPoint p, int paintCorner = 0)
        : QPoint(p)
        , paintCorner(paintCorner)
    {}

    int paintCorner;
};

} // anonymous namespace

void TerrainBrush::updateBrush(QPoint cursorPos, const QVector<QPoint> *list)
{
    mPaintX = cursorPos.x();
    mPaintY = cursorPos.y();

    // get the current tile layer
    TileLayer *currentLayer = currentTileLayer();
    Q_ASSERT(currentLayer);

    const QPoint layerPosition = currentLayer->position();
    int paintCorner = 0;

    cursorPos -= layerPosition;

    // if we are in vertex paint mode, the bottom right corner on the map will appear as an invalid tile offset...
    if (mBrushMode == PaintVertex) {
        if (cursorPos.x() == currentLayer->width()) {
            cursorPos.setX(cursorPos.x() - 1);
            paintCorner |= 1;
        }
        if (cursorPos.y() == currentLayer->height()) {
            cursorPos.setY(cursorPos.y() - 1);
            paintCorner |= 2;
        }
    }

    Tileset *terrainTileset = nullptr;
    int terrainId = -1;
    if (mTerrain) {
        terrainTileset = mTerrain->tileset();
        terrainId = mTerrain->id();
    }

    // create the tile stamp
    SharedTileLayer stamp = SharedTileLayer::create(QString(), 0, 0, 0, 0);

    // create a consideration list, and push the start points
    QVector<ConsiderationPoint> transitionList;

    if (list) { // if we were supplied a list of start points
        transitionList.reserve(list->size());
        for (QPoint p : *list)
            transitionList.append(p - layerPosition);
    } else {
        transitionList.append(ConsiderationPoint(cursorPos, paintCorner));
    }

    if (mMirrorDiagonally) {
        const int w = mapDocument()->map()->width();
        const int h = mapDocument()->map()->height();

        for (int i = 0, e = transitionList.size(); i < e; ++i) {
            const auto &p = transitionList.at(i);
            transitionList.append(ConsiderationPoint(QPoint(w - p.x() - 1,
                                                            h - p.y() - 1),
                                                     p.paintCorner ^ 3));
        }
    }

    // Determine the bounds of the affected area
    QRect bounds;
    for (ConsiderationPoint point : transitionList)
        bounds |= QRect(point, point);
    int margin = terrainTileset ? terrainTileset->maximumTerrainDistance() : 3;
    bounds.adjust(-margin, -margin, margin, margin);

    if (!mapDocument()->map()->infinite())
        bounds = bounds.intersected(currentLayer->rect().translated(-layerPosition));

    int initialTiles = transitionList.size();

    auto staggeredRenderer = dynamic_cast<StaggeredRenderer*>(mapDocument()->renderer());

    // produce terrain with transitions using a simple, relative naive approach (considers each tile once, and doesn't allow re-consideration if selection was bad)
    while (!transitionList.isEmpty()) {
        // get the next point in the consideration list
        const ConsiderationPoint p = transitionList.takeFirst();
        const int x = p.x();
        const int y = p.y();

        // if we have already considered this point, skip to the next
        // TODO: we might want to allow re-consideration if prior tiles... but not for now, this would risk infinite loops
        if (checked(*stamp, p))
            continue;

        // to support isometric staggered, make edges into variables
        QPoint upPoint(x, y - 1);
        QPoint bottomPoint(x, y + 1);
        QPoint leftPoint(x - 1, y);
        QPoint rightPoint(x + 1, y);

        if (staggeredRenderer) {
            upPoint = staggeredRenderer->topRight(x, y);
            bottomPoint = staggeredRenderer->bottomLeft(x, y);
            leftPoint = staggeredRenderer->topLeft(x, y);
            rightPoint = staggeredRenderer->bottomRight(x, y);
        }

        const Tile *tile = currentLayer->cellAt(p).tile();
        const unsigned currentTerrain = ::terrain(tile);

        // get the tileset for this tile
        Tileset *tileset = nullptr;
        if (terrainTileset) {
            // if we are painting a terrain, then we'll use the terrains tileset
            tileset = terrainTileset;
        } else if (tile) {
            // if we're erasing terrain, use the individual tiles tileset (to search for transitions)
            tileset = tile->tileset();
        } else {
            // no tile here and we're erasing terrain, not much we can do
            continue;
        }

        // calculate the ideal tile for this position
        unsigned preferredTerrain = 0xFFFFFFFF;
        unsigned mask = 0;

        if (initialTiles) {
            // for the initial tiles, we will insert the selected terrain and add the surroundings for consideration
            if (mBrushMode == PaintTile) {
                // set the whole tile to the selected terrain
                preferredTerrain = makeTerrain(terrainId);
                mask = 0xFFFFFFFF;
            } else {
                // Bail out if encountering a tile from a different tileset
                if (tile && tile->tileset() != tileset)
                    continue;

                // calculate the corner mask
                mask = 0xFF << (3 - p.paintCorner)*8;

                // mask in the selected terrain
                preferredTerrain = (currentTerrain & ~mask) | (terrainId << (3 - p.paintCorner)*8);
            }

            --initialTiles;

            // if there's nothing to paint... skip this tile
            if (preferredTerrain == currentTerrain && (!tile || tile->tileset() == tileset))
                continue;
        } else {
            // Bail out if encountering a tile from a different tileset
            if (tile && tile->tileset() != tileset)
                continue;

            // following tiles each need consideration against their surroundings
            preferredTerrain = currentTerrain;
            mask = 0;

            // depending which connections have been set, we update the preferred terrain of the tile accordingly
            if (contains(bounds, upPoint) && checked(*stamp, upPoint)) {
                preferredTerrain = (::terrain(stamp->cellAt(upPoint).tile()) << 16) | (preferredTerrain & 0x0000FFFF);
                mask |= 0xFFFF0000;
            }
            if (contains(bounds, bottomPoint) && checked(*stamp, bottomPoint)) {
                preferredTerrain = (::terrain(stamp->cellAt(bottomPoint).tile()) >> 16) | (preferredTerrain & 0xFFFF0000);
                mask |= 0x0000FFFF;
            }
            if (contains(bounds, leftPoint) && checked(*stamp, leftPoint)) {
                preferredTerrain = ((::terrain(stamp->cellAt(leftPoint).tile()) << 8) & 0xFF00FF00) | (preferredTerrain & 0x00FF00FF);
                mask |= 0xFF00FF00;
            }
            if (contains(bounds, rightPoint) && checked(*stamp, rightPoint)) {
                preferredTerrain = ((::terrain(stamp->cellAt(rightPoint).tile()) >> 8) & 0x00FF00FF) | (preferredTerrain & 0xFF00FF00);
                mask |= 0x00FF00FF;
            }
        }

        // find the most appropriate tile in the tileset
        // if all quadrants are set to 'no terrain', then the 'empty' tile is the only choice we can deduce
        Tile *paste = nullptr;
        if (preferredTerrain != 0xFFFFFFFF) {
            paste = findBestTile(*tileset, preferredTerrain, mask);
            if (!paste)
                continue;
        }

        // add tile to the brush
        Cell cell(paste);
        cell.setChecked(true);
        stamp->setCell(x, y, cell);

        // consider surrounding tiles if terrain constraints were not satisfied
        if (contains(bounds, upPoint) && !checked(*stamp, upPoint)) {
            const Tile *above = currentLayer->cellAt(upPoint).tile();
            if (topEdge(paste) != bottomEdge(above))
                transitionList.append(upPoint);
        }
        if (contains(bounds, bottomPoint) && !checked(*stamp, bottomPoint)) {
            const Tile *below = currentLayer->cellAt(bottomPoint).tile();
            if (bottomEdge(paste) != topEdge(below))
                transitionList.append(bottomPoint);
        }
        if (contains(bounds, leftPoint) && !checked(*stamp, leftPoint)) {
            const Tile *left = currentLayer->cellAt(leftPoint).tile();
            if (leftEdge(paste) != rightEdge(left))
                transitionList.append(leftPoint);
        }
        if (contains(bounds, rightPoint) && !checked(*stamp, rightPoint)) {
            const Tile *right = currentLayer->cellAt(rightPoint).tile();
            if (rightEdge(paste) != leftEdge(right))
                transitionList.append(rightPoint);
        }
    }

    // Translate to map coordinate space and normalize stamp
    QRegion brushRegion = stamp->region([] (const Cell &cell) { return cell.checked(); });
    brushRegion.translate(layerPosition);
    QRect brushRect = brushRegion.boundingRect();
    stamp->setPosition(brushRect.topLeft());
    stamp->resize(brushRect.size(), -brushRect.topLeft());

    // set the new tile layer as the brush
    brushItem()->setTileLayer(stamp, brushRegion);
}
