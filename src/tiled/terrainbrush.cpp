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

#include "brushitem.h"
#include "geometry.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "randompicker.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tile.h"
#include "terrain.h"

#include <math.h>
#include <QVector>
#include <climits>

using namespace Tiled;
using namespace Tiled::Internal;

TerrainBrush::TerrainBrush(QObject *parent)
    : AbstractTileTool(tr("Terrain Brush"),
                       QIcon(QLatin1String(
                               ":images/24x24/terrain-edit.png")),
                       QKeySequence(tr("T")),
                       parent)
    , mTerrain(nullptr)
    , mPaintX(0), mPaintY(0)
    , mIsActive(false)
    , mBrushBehavior(Free)
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

    const QPoint &position = tilePosition();

    if (!tileLayer->contains(position))
        return;

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

    if (!tileLayer->bounds().intersects(stamp->bounds()))
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(), tileLayer,
                                               stamp->x(), stamp->y(),
                                               stamp, brushItem()->tileRegion());
    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    mapDocument()->emitRegionEdited(brushItem()->tileRegion(), tileLayer);
}

static Tile *findBestTile(const Tileset &tileset, unsigned terrain, unsigned considerationMask)
{
    // we should have hooked 0xFFFFFFFF terrains outside this function
    Q_ASSERT(terrain != 0xFFFFFFFF);

    RandomPicker<Tile*> matches;
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

void TerrainBrush::updateBrush(QPoint cursorPos, const QVector<QPoint> *list)
{
    mPaintX = cursorPos.x();
    mPaintY = cursorPos.y();

    // get the current tile layer
    TileLayer *currentLayer = currentTileLayer();
    Q_ASSERT(currentLayer);

    int layerWidth = currentLayer->width();
    int layerHeight = currentLayer->height();
    int numTiles = layerWidth * layerHeight;
    int paintCorner = 0;

    // if we are in vertex paint mode, the bottom right corner on the map will appear as an invalid tile offset...
    if (mBrushMode == PaintVertex) {
        if (cursorPos.x() == layerWidth) {
            cursorPos.setX(cursorPos.x() - 1);
            paintCorner |= 1;
        }
        if (cursorPos.y() == layerHeight) {
            cursorPos.setY(cursorPos.y() - 1);
            paintCorner |= 2;
        }
    }

    // if the cursor is outside of the map, bail out
    if (!currentLayer->bounds().contains(cursorPos)) {
        brushItem()->clear();
        return;
    }

    Tileset *terrainTileset = nullptr;
    int terrainId = -1;
    if (mTerrain) {
        terrainTileset = mTerrain->tileset();
        terrainId = mTerrain->id();
    }

    // allocate a buffer to build the terrain tilemap (TODO: this could be retained per layer to save regular allocation)
    Tile **newTerrain = new Tile*[numTiles];

    // allocate a buffer of flags for each tile that may be considered (TODO: this could be retained per layer to save regular allocation)
    char *checked = new char[numTiles];
    memset(checked, 0, numTiles);

    // create a consideration list, and push the start points
    QList<QPoint> transitionList;
    int initialTiles = 0;

    if (list) {
        // if we were supplied a list of start points
        foreach (const QPoint &p, *list) {
            transitionList.append(p);
            ++initialTiles;
        }
    } else {
        transitionList.append(cursorPos);
        initialTiles = 1;
    }

    QRect brushRect(cursorPos, cursorPos);

    // produce terrain with transitions using a simple, relative naive approach (considers each tile once, and doesn't allow re-consideration if selection was bad)
    while (!transitionList.isEmpty()) {
        // get the next point in the consideration list
        QPoint p = transitionList.takeFirst();
        int x = p.x(), y = p.y();
        int i = y*layerWidth + x;

        // if we have already considered this point, skip to the next
        // TODO: we might want to allow re-consideration if prior tiles... but not for now, this would risk infinite loops
        if (checked[i])
            continue;

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
                mask = 0xFF << (3 - paintCorner)*8;

                // mask in the selected terrain
                preferredTerrain = (currentTerrain & ~mask) | (terrainId << (3 - paintCorner)*8);
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
            if (y > 0 && checked[i - layerWidth]) {
                preferredTerrain = (::terrain(newTerrain[i - layerWidth]) << 16) | (preferredTerrain & 0x0000FFFF);
                mask |= 0xFFFF0000;
            }
            if (y < layerHeight - 1 && checked[i + layerWidth]) {
                preferredTerrain = (::terrain(newTerrain[i + layerWidth]) >> 16) | (preferredTerrain & 0xFFFF0000);
                mask |= 0x0000FFFF;
            }
            if (x > 0 && checked[i - 1]) {
                preferredTerrain = ((::terrain(newTerrain[i - 1]) << 8) & 0xFF00FF00) | (preferredTerrain & 0x00FF00FF);
                mask |= 0xFF00FF00;
            }
            if (x < layerWidth - 1 && checked[i + 1]) {
                preferredTerrain = ((::terrain(newTerrain[i + 1]) >> 8) & 0x00FF00FF) | (preferredTerrain & 0xFF00FF00);
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
        newTerrain[i] = paste;
        checked[i] = true;

        // expand the brush rect to fit the edit set
        brushRect |= QRect(p, p);

        // consider surrounding tiles if terrain constraints were not satisfied
        if (y > 0 && !checked[i - layerWidth]) {
            const Tile *above = currentLayer->cellAt(x, y - 1).tile();
            if (topEdge(paste) != bottomEdge(above))
                transitionList.append(QPoint(x, y - 1));
        }
        if (y < layerHeight - 1 && !checked[i + layerWidth]) {
            const Tile *below = currentLayer->cellAt(x, y + 1).tile();
            if (bottomEdge(paste) != topEdge(below))
                transitionList.append(QPoint(x, y + 1));
        }
        if (x > 0 && !checked[i - 1]) {
            const Tile *left = currentLayer->cellAt(x - 1, y).tile();
            if (leftEdge(paste) != rightEdge(left))
                transitionList.append(QPoint(x - 1, y));
        }
        if (x < layerWidth - 1 && !checked[i + 1]) {
            const Tile *right = currentLayer->cellAt(x + 1, y).tile();
            if (rightEdge(paste) != leftEdge(right))
                transitionList.append(QPoint(x + 1, y));
        }
    }

    // create a stamp for the terrain block
    QRegion brushRegion;

    // LUCA-TODO: The preview should have the same tile size as the layer
    SharedTileLayer stamp = SharedTileLayer(new TileLayer(QString(),
                                                          brushRect.left(),
                                                          brushRect.top(),
                                                          brushRect.width(),
                                                          brushRect.height(),
                                                          currentLayer->tileWidth(),
                                                          currentLayer->tileHeight()));

    for (int y = brushRect.top(); y <= brushRect.bottom(); ++y) {
        for (int x = brushRect.left(); x <= brushRect.right(); ++x) {
            int i = y * layerWidth + x;
            if (!checked[i])
                continue;

            stamp->setCell(x - brushRect.left(),
                           y - brushRect.top(),
                           Cell(newTerrain[i]));

            // detect the affected region in ranges, which makes things faster
            const int rangeStart = x;

            for (++x; x <= brushRect.right() + 1; ++x) {
                i = y * layerWidth + x;
                if (x == brushRect.right() + 1 || !checked[i]) {
                    const int rangeEnd = x;
                    brushRegion += QRect(rangeStart, y,
                                         rangeEnd - rangeStart, 1);
                    break;
                } else {
                    stamp->setCell(x - brushRect.left(),
                                   y - brushRect.top(),
                                   Cell(newTerrain[i]));
                }
            }
        }
    }

    // set the new tile layer as the brush
    brushItem()->setTileLayer(stamp, brushRegion);

    delete[] checked;
    delete[] newTerrain;
}
