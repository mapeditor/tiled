/*
 * automap.cpp
 * Copyright 2010, Stefan Beller, stefanbeller@googlemail.com
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

#include "automap.h"

#include "addremovelayer.h"
#include "addremovetileset.h"
#include "changeproperties.h"
#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilepainter.h"
#include "tileset.h"

#include <QMessageBox>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

AutoMapper::AutoMapper(MapDocument *workingDocument, Map *rulesMap)
    : mMapDocument(workingDocument),
      mMapWork(workingDocument->map()),
      mMapRules(rulesMap)
{
}

bool AutoMapper::setupLayers()
{
    cleanUpLayers();

    mLayerRuleSet     = findTileLayer(mMapRules, QLatin1String("ruleset"));
    mLayerRuleRegions = findTileLayer(mMapRules, QLatin1String("ruleregions"));
    mLayerSet         = findTileLayer(mMapWork, QLatin1String("set"));

    QString prefix = QLatin1String("rule_");
    foreach (Layer *layer, mMapRules->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            if (layer->name().startsWith(prefix, Qt::CaseInsensitive)) {
                QString name = layer->name();
                name.remove(0, prefix.length());
                TileLayer *t = findTileLayer(mMapWork, name);
                if (!t) {
                    const int index = mMapWork->layerCount();
                    t = new TileLayer(name, 0, 0,
                                      mMapWork->width(), mMapWork->height());
                    mMapDocument->undoStack()->push(
                                new AddLayer(mMapDocument, index, t));
                    mMapDocument->setCurrentLayer(index);
                    mAddedTileLayers.append(t->name());
                }
                mLayerList << QPair<TileLayer*, TileLayer*>(tileLayer, t);
            }
        }
    }

    QString error;

    if (!mLayerRuleRegions)
        error += tr("No ruleRegions layer found!") + QLatin1Char('\n');
    if (!mLayerSet)
        error += tr("No set layer found!") + QLatin1Char('\n');
    if (!mLayerRuleSet)
        error += tr("No ruleSet layer found!") + QLatin1Char('\n');

    if (!error.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(error);
        msgBox.exec();
        return false;
    }

    return true;
}

bool AutoMapper::setupAll()
{
    if (!setupLayers())
        return false;

    if (!setupRules())
        return false;

    if (!setupTilesets(mMapRules, mMapWork))
        return false;

    return true;
}

QList<QString> AutoMapper::getTouchedLayers() const
{
    QList<QString> ret;
    QList<QPair<TileLayer*, TileLayer*> >::const_iterator j;

    for (j = mLayerList.constBegin(); j != mLayerList.constEnd(); ++j)
        ret << j->second->name();

    return ret;
}

TileLayer *AutoMapper::findTileLayer(Map *map, const QString &name)
{
    TileLayer *ret = 0;
    QString error;

    foreach (Layer *layer, map->layers()) {
        if (layer->name().compare(name, Qt::CaseInsensitive) == 0) {
            if (TileLayer *tileLayer = layer->asTileLayer()) {
                if (ret)
                    error = tr("Multiple layers %1 found!").arg(name);
                ret = tileLayer;
            }
        }
    }

    if (!error.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(error);
        msgBox.exec();
    }

    return ret;
}

bool AutoMapper::isPartOfExistingRule(QPoint p) const
{
    foreach (const QRegion &region, mRules)
        if (region.contains(p))
            return true;

    return false;
}

QRegion AutoMapper::createRule(int x, int y) const
{
    QRegion ret(x, y, 1, 1);
    QList<QPoint> addPoints;
    Tile *match = mLayerRuleRegions->tileAt(x, y);
    addPoints.append(QPoint(x, y));

    while (!addPoints.empty()) {
        const QPoint current = addPoints.takeFirst();
        x = current.x();
        y = current.y();
        if (mLayerRuleRegions->tileAt(x - 1, y) == match
                && !ret.contains(QPoint(x - 1, y))) {
            ret += QRegion(x - 1, y, 1, 1);
            addPoints.append(QPoint(x - 1, y));
        }
        if (mLayerRuleRegions->tileAt(x + 1, y) == match
                && !ret.contains(QPoint(x + 1, y))) {
            ret += QRegion(x + 1, y, 1, 1);
            addPoints.append(QPoint(x + 1, y));
        }
        if (mLayerRuleRegions->tileAt(x, y - 1) == match
                && !ret.contains(QPoint(x, y - 1))) {
            ret += QRegion(x, y - 1, 1, 1);
            addPoints.append(QPoint(x, y - 1));
        }
        if (mLayerRuleRegions->tileAt(x, y + 1) == match
                && !ret.contains(QPoint(x, y + 1))) {
            ret += QRegion(x, y + 1, 1, 1);
            addPoints.append(QPoint(x, y + 1));
        }
    }

    return ret;
}

bool AutoMapper::setupRules()
{
    mRules.clear();

    for (int y = 1; y < mMapRules->height(); y++ ) {
        for (int x = 1; x < mMapRules->width();  x++ ) {
            if (mLayerRuleRegions->tileAt(x, y)
                    && !isPartOfExistingRule(QPoint(x, y))) {
                mRules << createRule(x, y);
            }
        }
    }
    return true;
}


static Tileset *findSimilarTileset(const Tileset *tileset,
                                   const QList<Tileset*> &tilesets)
{
    foreach (Tileset *candidate, tilesets) {
        if (candidate != tileset
            && candidate->imageSource() == tileset->imageSource()
            && candidate->tileWidth() == tileset->tileWidth()
            && candidate->tileHeight() == tileset->tileHeight()
            && candidate->tileSpacing() == tileset->tileSpacing()
            && candidate->margin() == tileset->margin()) {
                return candidate;
        }
    }
    return 0;
}

bool AutoMapper::setupTilesets(Map *src, Map *dst)
{
    QList<Tileset*> existingTilesets = dst->tilesets();

    // Add tilesets that are not yet part of dst map
    foreach (Tileset *tileset, src->tilesets()) {
        if (existingTilesets.contains(tileset))
            continue;

        QUndoStack *undoStack = mMapDocument->undoStack();

        Tileset *replacement = findSimilarTileset(tileset, existingTilesets);
        if (!replacement) {
            mAddedTilesets.append(tileset);
            undoStack->push(new AddTileset(mMapDocument, tileset));
            continue;
        }

        // Merge the tile properties
        const int sharedTileCount = qMin(tileset->tileCount(),
                                         replacement->tileCount());
        for (int i = 0; i < sharedTileCount; ++i) {
            Tile *replacementTile = replacement->tileAt(i);
            Properties properties = replacementTile->properties();
            properties.merge(tileset->tileAt(i)->properties());

            undoStack->push(new ChangeProperties(tr("Tile"),
                                                 replacementTile,
                                                 properties));
        }
        src->replaceTileset(tileset, replacement);
        delete tileset;
    }
    return true;
}

void AutoMapper::copyRegion(TileLayer *srcLayer, int srcX, int srcY,
                            int width, int height,
                            TileLayer *dstLayer, int dstX, int dstY)
{
    TilePainter tp(mMapDocument, dstLayer);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (Tile *t = srcLayer->tileAt(srcX + x, srcY + y))
                tp.setTile(dstX + x, dstY + y, t);
        }
    }
}

void AutoMapper::copyMapRegion(const QRegion &region, QPoint offset,
                               const QList< QPair<TileLayer*,TileLayer*> > &layerTranslation)
{
    QList< QPair<TileLayer*, TileLayer*> >::const_iterator lr_i;
    for (lr_i = layerTranslation.begin();
         lr_i != layerTranslation.end();
         ++lr_i) {
        foreach (QRect rect, region.rects()) {
            copyRegion(lr_i->first, rect.x(), rect.y(),
                       rect.width(), rect.height(),
                       lr_i->second, rect.x() + offset.x(), rect.y() + offset.y());
        }
    }
}

static QList<Tile*> tilesInRegion(TileLayer *l, const QRegion &r)
{
    QList<Tile*> tiles;
    foreach (QRect rect, r.rects()) {
        for (int x = rect.left(); x <= rect.right(); x++) {
            for (int y = rect.top(); y <= rect.bottom(); y++) {
                Tile *t = l->tileAt(x, y);
                if (!tiles.contains(t))
                    tiles.append(t);
            }
        }
    }
    return tiles;
}

/**
 * This compares two tile layers.
 * The first tile layer is examined at QRegion r1
 * The second tile layer is examined at r1 + offset
 * When at both places are tiles, these will be compared.
 *
 * If there is no tile in one layer, in the other layer there must be a
 * tile, which must not occur in the region of the first layer.
 *
 * @return bool, if the tile layers are the same at the specific regions
 */
static bool compareLayers(TileLayer *l1, TileLayer *l2,
                          const QRegion &r1, QPoint offset)
{
    QList<Tile*> tiles1 = tilesInRegion(l1, r1);
    QRegion r2(r1);
    r2.translate(offset.x(), offset.y());
    QList<Tile*> tiles2 = tilesInRegion(l1, r2);

    foreach (QRect rect, r1.rects()) {
        for (int x = rect.left(); x <= rect.right(); x++) {
            for (int y = rect.top(); y <= rect.bottom(); y++) {
                if (!(l1->contains(x, y) &&
                      l2->contains(x + offset.x(), y + offset.y())))
                    return false;
                Tile *t1 = l1->tileAt(x, y);
                Tile *t2 = l2->tileAt(x + offset.x(), y + offset.y());
                if (t1 && t2 && t1 != t2)
                    return false;
                if (!t1 && tiles1.contains(t2))
                    return false;
                if (!t2 && tiles2.contains(t1))
                    return false;
            }
        }
    }
    return true;
}

void AutoMapper::applyRule(const QRegion &rule)
{
    int x = - rule.boundingRect().left();
    int y = - rule.boundingRect().top();
    const int max_x = x + mMapWork->width();
    const int max_y = y + mMapWork->height();

    while (y <= max_y) {
        while (x <= max_x) {
            if (compareLayers(mLayerRuleSet, mLayerSet, rule, QPoint(x, y)))
                copyMapRegion(rule, QPoint(x, y), mLayerList);
            x++;
        }
        x = -rule.boundingRect().left();
        y++;
    }
}

void AutoMapper::autoMap()
{
    foreach (const QRegion &rule, mRules)
        applyRule(rule);
}

void AutoMapper::cleanUpLayers()
{
    foreach (const QString &t, mAddedTileLayers) {
        const int layerindex = mMapWork->indexOfLayer(t);
        if (layerindex != -1) {
            TileLayer *t = mMapWork->layerAt(layerindex)->asTileLayer();
            if (t->isEmpty()) {
                mMapDocument->undoStack()->push(
                            new RemoveLayer(mMapDocument, layerindex));
            }
        }
    }
    mLayerList.clear();
}

void AutoMapper::cleanUpTilesets()
{
    foreach (Tileset *t, mAddedTilesets) {
        if (mMapWork->isTilesetUsed(t))
            continue;

        const int layerIndex = mMapWork->indexOfTileset(t);
        if (layerIndex != -1) {
            QUndoCommand *cmd = new RemoveTileset(mMapDocument, layerIndex, t);
            mMapDocument->undoStack()->push(cmd);
        }
    }
}

AutoMapper::~AutoMapper()
{
    cleanUpLayers();
    cleanUpTilesets();
}


AutomaticMapping::AutomaticMapping(MapDocument *workingDocument, Map *rules)
    : mMapDocument(workingDocument)
{
    AutoMapper autoMapper(workingDocument, rules);
    if (autoMapper.setupAll()) {
        Map *map = mMapDocument->map();
        foreach (const QString &layerName, autoMapper.getTouchedLayers()) {
            const int layerindex = map->indexOfLayer(layerName);
            mLayersBefore << map->layerAt(layerindex)->clone();
        }
        autoMapper.autoMap();
        foreach (const QString &layerName, autoMapper.getTouchedLayers()) {
            const int layerindex = map->indexOfLayer(layerName);
            // layerindex exists, because AutoMapper is still alive, dont check
            mLayersAfter << map->layerAt(layerindex)->clone();
        }
    }
}

AutomaticMapping::~AutomaticMapping()
{
    QList<Layer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i)
        delete *i;
    for (i = mLayersBefore.begin(); i != mLayersBefore.end(); ++i)
        delete *i;
}

void AutomaticMapping::undo()
{
    Map *map = mMapDocument->map();
    QList<Layer*>::iterator i;
    for (i = mLayersBefore.begin();i!=mLayersBefore.end(); ++i) {
        const int layerindex = map->indexOfLayer((*i)->name());
        if (layerindex != -1)
            //just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex, (*i)->clone());
    }
}
void AutomaticMapping::redo()
{
    Map *map = mMapDocument->map();
    QList<Layer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i) {
        const int layerindex = (map->indexOfLayer((*i)->name()));
        if (layerindex != -1)
            // just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex, (*i)->clone());
    }
}

Layer *AutomaticMapping::swapLayer(int layerIndex, Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayer();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(layerIndex);
    layerModel->insertLayer(layerIndex, layer);

    if (layerIndex == currentIndex)
        mMapDocument->setCurrentLayer(layerIndex);

    return replaced;
}
