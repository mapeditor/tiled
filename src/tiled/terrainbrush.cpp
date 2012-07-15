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
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
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
    , mTerrain(NULL)
    , mPaintX(0), mPaintY(0)
    , mOffsetX(0), mOffsetY(0)
    , mBrushBehavior(Free)
    , mLineReferenceX(0)
    , mLineReferenceY(0)
{
    setBrushMode(PaintTile);
}

TerrainBrush::~TerrainBrush()
{
}

void TerrainBrush::tilePositionChanged(const QPoint &pos)
{
    switch (mBrushBehavior) {
    case Paint:
    {
        int x = mPaintX;
        int y = mPaintY;
        foreach (const QPoint &p, pointsOnLine(x, y, pos.x(), pos.y())) {
            updateBrush(p);
            doPaint(true, p.x(), p.y());
        }
        // HACK-ish: because the line may traverse in the reverse direction, updateBrush() leaves these at the line start point
        mPaintX = pos.x();
        mPaintY = pos.y();
        break;
    }
    case LineStartSet:
    {
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
            doPaint(false, mPaintX, mPaintY);
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
    if (modifiers & Qt::ShiftModifier) {
        mBrushBehavior = Line;
    } else {
        mBrushBehavior = Free;
    }

    setBrushMode((modifiers & Qt::ControlModifier) ? PaintVertex : PaintTile);
    updateBrush(tilePosition());
}

void TerrainBrush::languageChanged()
{
    setName(tr("Terain Brush"));
    setShortcut(QKeySequence(tr("T")));
}

static Terrain *firstTerrain(MapDocument *mapDocument)
{
    if (!mapDocument)
        return 0;

    foreach (Tileset *tileset, mapDocument->map()->tilesets())
        if (tileset->terrainCount() > 0)
            return tileset->terrain(0);

    return 0;
}

void TerrainBrush::mapDocumentChanged(MapDocument *oldDocument,
                                      MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    // Reset the brush, since it probably became invalid
    brushItem()->setTileLayer(0);

    // Don't use setTerrain since we do not want to update the brush right now
    mTerrain = firstTerrain(newDocument);
}

void TerrainBrush::setTerrain(const Terrain *terrain)
{
    if (mTerrain == terrain)
        return;

    mTerrain = terrain;

    updateBrush(tilePosition());
}

void TerrainBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint(false, mPaintX, mPaintY);
}

void TerrainBrush::capture()
{
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    // TODO: we need to know which corner the mouse is closest to...

    const Cell &cell = tileLayer->cellAt(tilePosition());
    Terrain *t = cell.tile->terrainAtCorner(0);
    setTerrain(t);
}

void TerrainBrush::doPaint(bool mergeable, int whereX, int whereY)
{
    TileLayer *stamp = brushItem()->tileLayer();

    if (!stamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    whereX -= mOffsetX;
    whereY -= mOffsetY;

    if (!tileLayer->bounds().intersects(QRect(whereX, whereY, stamp->width(), stamp->height())))
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(), tileLayer, whereX, whereY, stamp);
    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    mapDocument()->emitRegionEdited(brushItem()->tileRegion(), tileLayer);
}

static inline unsigned int makeTerrain(int t)
{
    t &= 0xFF;
    return t << 24 | t << 16 | t << 8 | t;
}

static inline unsigned int makeTerrain(int tl, int tr, int bl, int br)
{
    return (tl & 0xFF) << 24 | (tr & 0xFF) << 16 | (bl & 0xFF) << 8 | (br & 0xFF);
}

Tile *TerrainBrush::findBestTile(Tileset *tileset, unsigned int terrain, unsigned int considerationMask)
{
    // we should have hooked 0xFFFFFFFF terrains outside this function
    Q_ASSERT(terrain != 0xFFFFFFFF);

    // if all quadrants are set to 'no terrain', then the 'empty' tile is the only choice we can deduce
    if (terrain == 0xFFFFFFFF)
        return NULL;

    QList<Tile*> matches;
    int penalty = INT_MAX;

    // TODO: this is a slow linear search, perhaps we could use a better find algorithm...
    int tileCount = tileset->tileCount();
    for (int i = 0; i < tileCount; ++i) {
        Tile *t = tileset->tileAt(i);
        if ((t->terrain() & considerationMask) != (terrain & considerationMask))
            continue;

        // calculate the tile transition penalty based on shortest distance to target terrain type
        int tr = tileset->terrainTransitionPenalty(t->terrain() >> 24, terrain >> 24);
        int tl = tileset->terrainTransitionPenalty((t->terrain() >> 16) & 0xFF, (terrain >> 16) & 0xFF);
        int br = tileset->terrainTransitionPenalty((t->terrain() >> 8) & 0xFF, (terrain >> 8) & 0xFF);
        int bl = tileset->terrainTransitionPenalty(t->terrain() & 0xFF, terrain & 0xFF);

        // if there is no path to the destination terrain, this isn't a useful transition
        if (tr < 0 || tl < 0 || br < 0 || bl < 0)
            continue;

        // add tile to the candidate list
        int transitionPenalty = tr + tl + br + bl;
        if (transitionPenalty <= penalty) {
            if (transitionPenalty < penalty)
                matches.clear();
            penalty = transitionPenalty;

            matches.push_back(t);
        }
    }

    // choose a candidate at random, with consideration for terrain probability
    if (!matches.isEmpty()) {
        float random = ((float)rand() / RAND_MAX) * 100.f;
        float total = 0, unassigned = 0;

        // allow the tiles with assigned probability to take their share
        for (int i = 0; i < matches.size(); ++i) {
            float probability = matches[i]->terrainProbability();
            if (probability < 0.f) {
                ++unassigned;
                continue;
            }
            if (random < total + probability)
                return matches[i];
            total += probability;
        }

        // divide the remaining percentile by the numer of unassigned tiles
        float remainingShare = (100.f - total) / (float)unassigned;
        for (int i = 0; i < matches.size(); ++i) {
            if (matches[i]->terrainProbability() >= 0.f)
                continue;
            if (random < total + remainingShare)
                return matches[i];
            total += remainingShare;
        }
    }

    // TODO: conveniently, the NULL tile doesn't currently work, but when it does, we need to signal a failure to find any matches some other way
    return NULL;
}

void TerrainBrush::updateBrush(QPoint cursorPos, const QVector<QPoint> *list)
{
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
    if (!currentLayer->bounds().contains(cursorPos))
        return;

    // TODO: this seems like a problem... there's nothing to say that 2 adjacent tiles are from the same tileset, or have any relation to eachother...
    Tileset *terrainTileset = mTerrain ? mTerrain->tileset() : NULL;

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
            transitionList.push_back(p);
            ++initialTiles;
        }
    } else {
        transitionList.push_back(cursorPos);
        initialTiles = 1;
    }

    QRect brushRect(cursorPos, cursorPos);

    // produce terrain with transitions using a simple, relative naive approach (considers each tile once, and doesn't allow re-consideration if selection was bad)
    while (!transitionList.isEmpty()) {
        // get the next point in the consideration list
        QPoint p = transitionList.front();
        transitionList.pop_front();
        int x = p.x(), y = p.y();
        int i = y*layerWidth + x;

        // if we have already considered this point, skip to the next
        // TODO: we might want to allow re-consideration if prior tiles... but not for now, this would risk infinite loops
        if (checked[i])
            continue;

        const Tile *tile = currentLayer->cellAt(p).tile;

        // get the tileset for this tile
        Tileset *tileset = NULL;
        if (terrainTileset) // if we are painting a terrain, then we'll use the terrains tileset
            tileset = terrainTileset;
        else if(tile) // if we're erasing terrain, use the individual tiles tileset (to search for transitions)
            tileset = tile->tileset();

        // calculate the ideal tile for this position
        unsigned int preferredTerrain, mask;
        Tile *paste = NULL;

        if (initialTiles) {
            // for the initial tiles, we will insert the selected terrain and add the surroundings for consideration
            unsigned int currentTerrain = tile->terrain();

            if (mBrushMode == PaintTile) {
                // set the whole tile to the selected terrain
                preferredTerrain = makeTerrain(mTerrain->id());
                mask = 0xFFFFFFFF;
            } else {
                // calculate the corner mask
                mask = 0xFF << (3 - paintCorner)*8;

                // mask in the selected terrain
                preferredTerrain = (currentTerrain & ~mask) | (mTerrain->id() << (3 - paintCorner)*8);
            }

            --initialTiles;

            // if there's nothing to paint... skip this tile
            if (preferredTerrain == currentTerrain)
                continue;
        } else {
            // following tiles each need consideration against their surroundings
            preferredTerrain = tile->terrain();
            mask = 0;

            // depending which connections have been set, we update the preferred terrain of the tile accordingly
            if (y > 0 && checked[i - layerWidth]) {
                preferredTerrain = (newTerrain[i - layerWidth]->terrain() << 16) | (preferredTerrain & 0x0000FFFF);
                mask |= 0xFFFF0000;
            }
            if (y < layerHeight - 1 && checked[i + layerWidth]) {
                preferredTerrain = (newTerrain[i + layerWidth]->terrain() >> 16) | (preferredTerrain & 0xFFFF0000);
                mask |= 0x0000FFFF;
            }
            if (x > 0 && checked[i - 1]) {
                preferredTerrain = ((newTerrain[i - 1]->terrain() << 8) & 0xFF00FF00) | (preferredTerrain & 0x00FF00FF);
                mask |= 0xFF00FF00;
            }
            if (x < layerWidth - 1 && checked[i + 1]) {
                preferredTerrain = ((newTerrain[i + 1]->terrain() >> 8) & 0x00FF00FF) | (preferredTerrain & 0xFF00FF00);
                mask |= 0x00FF00FF;
            }
        }

        // find the most appropriate tile in the tileset
        if (preferredTerrain != 0xFFFFFFFF) {
            paste = findBestTile(tileset, preferredTerrain, mask);
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
            const Tile *above = currentLayer->cellAt(x, y - 1).tile;
            if (paste->topEdge() != above->bottomEdge())
                transitionList.push_back(QPoint(x, y - 1));
        }
        if (y < layerHeight - 1 && !checked[i + layerWidth]) {
            const Tile *below = currentLayer->cellAt(x, y + 1).tile;
            if (paste->bottomEdge() != below->topEdge())
                transitionList.push_back(QPoint(x, y + 1));
        }
        if (x > 0 && !checked[i - 1]) {
            const Tile *left = currentLayer->cellAt(x - 1, y).tile;
            if (paste->leftEdge() != left->rightEdge())
                transitionList.push_back(QPoint(x - 1, y));
        }
        if (x < layerWidth - 1 && !checked[i + 1]) {
            const Tile *right = currentLayer->cellAt(x + 1, y).tile;
            if (paste->rightEdge() != right->leftEdge())
                transitionList.push_back(QPoint(x + 1, y));
        }
    }

    // create a stamp for the terrain block
    TileLayer *stamp = new TileLayer(QString(), 0, 0, brushRect.width(), brushRect.height());

    for (int y = brushRect.top(); y <= brushRect.bottom(); ++y) {
        for (int x = brushRect.left(); x <= brushRect.right(); ++x) {
            int i = y*layerWidth + x;
            if (!checked[i])
                continue;

            Tile *tile = newTerrain[i];
            if (tile)
                stamp->setCell(x - brushRect.left(), y - brushRect.top(), Cell(tile));
            else {
                // TODO: we need to do something to erase tiles where checked[i] is true, and newTerrain[i] is NULL
                // is there an eraser stamp? investigate how the eraser works...
            }
        }
    }

    // set the new tile layer as the brush
    brushItem()->setTileLayer(stamp);

    delete[] checked;
    delete[] newTerrain;

/*
    const QPoint tilePos = tilePosition();

    if (!brushItem()->tileLayer()) {
        brushItem()->setTileRegion(QRect(tilePos, QSize(1, 1)));
    }
*/

    brushItem()->setTileLayerPosition(QPoint(brushRect.left(), brushRect.top()));

    mPaintX = cursorPos.x();
    mPaintY = cursorPos.y();
    mOffsetX = cursorPos.x() - brushRect.left();
    mOffsetY = cursorPos.y() - brushRect.top();
}
