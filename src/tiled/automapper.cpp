/*
 * automapper.cpp
 * Copyright 2010-2011, Stefan Beller, stefanbeller@googlemail.com
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

#include "automapper.h"

#include "addremovelayer.h"
#include "addremovetileset.h"
#include "changeproperties.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "utils.h"

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;

/*
 * About the order of the methods in this file.
 * The Automapper class has 3 bigger public functions, that is
 * prepareLoad(), prepareAutoMap() and autoMap().
 * These three functions make use of lots of different private methods, which
 * are put directly below each of these functions.
 */

AutoMapper::AutoMapper(MapDocument *workingDocument, QString setlayer)
    : mMapDocument(workingDocument)
    , mMapWork(workingDocument ? workingDocument->map() : 0)
    , mMapRules(0)
    , mLayerRuleRegions(0)
    , mSetLayer(setlayer)
    , mSetLayerIndex(-1)
    , mDeleteTiles(false)
    , mAutoMappingRadius(0)
    , mNoOverlappingRules(false)
{
}

AutoMapper::~AutoMapper()
{
    cleanUpRulesMap();
}

QSet<QString> AutoMapper::getTouchedLayers() const
{
    return mTouchedLayers.toSet();
}

bool AutoMapper::prepareLoad(Map *rules, const QString &rulePath)
{
    mError.clear();
    mWarning.clear();

    if (!setupMapDocumentLayers())
        return false;

    if (!setupRuleMapProperties(rules, rulePath))
        return false;

    if (!setupRuleMapTileLayers())
        return false;

    if (!setupRulesUsedCheck())
        return false;

    if (!setupRuleList())
        return false;

    return true;
}

bool AutoMapper::setupMapDocumentLayers()
{
    Q_ASSERT(mSetLayerIndex == -1);
    mSetLayerIndex = mMapWork->indexOfLayer(mSetLayer);

    if (mSetLayerIndex == -1)
        return false;

    return true;
}

bool AutoMapper::setupRuleMapProperties(Map *rules, const QString &rulePath)
{
    Q_ASSERT(!mMapRules);

    mMapRules = rules;
    mRulePath = rulePath;

    Properties properties = rules->properties();
    foreach (QString key, properties.keys()) {
        QVariant value = properties.value(key);
        bool raiseWarning = true;
        if (key.toLower() == QLatin1String("deletetiles")) {
            if (value.canConvert(QVariant::Bool)) {
                mDeleteTiles = value.toBool();
                raiseWarning = false;
            }
        } else if (key.toLower() == QLatin1String("automappingradius")) {
            if (value.canConvert(QVariant::Int)) {
                mAutoMappingRadius = value.toInt();
                raiseWarning = false;
            }
        } else if (key.toLower() == QLatin1String("nooverlappingrules")) {
            if (value.canConvert(QVariant::Bool)) {
                mNoOverlappingRules = value.toBool();
                raiseWarning = false;
            }
        }
        if (raiseWarning)
            mWarning += tr("%1: Property %2 = %3 does not make sense. "
                           "Ignoring this property.")
                    .arg(rulePath, key, value.toString()) + QLatin1Char('\n');
    }
    return true;
}

bool AutoMapper::setupRuleMapTileLayers()
{
    Q_ASSERT(mLayerList.isEmpty());
    Q_ASSERT(!mLayerRuleRegions);
    Q_ASSERT(mLayerRuleSets.isEmpty());
    Q_ASSERT(mLayerRuleNotSets.isEmpty());
    Q_ASSERT(mAddedTilesets.isEmpty());

    QString prefix = QLatin1String("rule");

    foreach (Layer *layer, mMapRules->layers()) {
        TileLayer *tileLayer = layer->asTileLayer();
        if (!tileLayer)
            continue;

        if (!tileLayer->name().startsWith(prefix, Qt::CaseInsensitive)) {
            mWarning += tr("Layer %1 found in automapping rules."
                           "Did you mean %2_%1? Ignoring that layer!")
                        .arg(tileLayer->name(), prefix) + QLatin1Char('\n');
            continue;
        }

        // strip leading prefix, to make handling better
        QString layername = tileLayer->name();
        layername.remove(0, prefix.length());

        if (layername.startsWith(QLatin1String("set"),
                                 Qt::CaseInsensitive)) {
            mLayerRuleSets.append(tileLayer);
            continue;
        }

        if (layername.startsWith(QLatin1String("notset"),
                                 Qt::CaseInsensitive)) {
            mLayerRuleNotSets.append(tileLayer);
            continue;
        }

        if (layername.startsWith(QLatin1String("regions"),
                                 Qt::CaseInsensitive)) {
            mLayerRuleRegions = tileLayer;
            continue;
        }

        int nameStartPosition = layername.indexOf(QLatin1Char('_')) + 1;
        QString group = layername.left(nameStartPosition) ;

        QString name = layername.right(layername.size() - nameStartPosition);
        mTouchedLayers.append(name);
        int indexOfLayer = mMapWork->indexOfLayer(name);

        bool found = false;
        foreach (IndexByTileLayer *translationTable, mLayerList) {
            const QString storedName = translationTable->keys().at(0)->name();
            // check if the group name is at the right position! index != -1
            // does not work, since the group name might be in the layer name
            if (storedName.indexOf(group) == prefix.length()) {
                translationTable->insert(tileLayer, indexOfLayer);
                found = true;
                break;
            }
        }
        if (!found) {
            mLayerList.append(new IndexByTileLayer());
            mLayerList.last()->insert(tileLayer, indexOfLayer);
        }
    }

    QString error;

    if (!mLayerRuleRegions)
        error += tr("No ruleRegions layer found!") + QLatin1Char('\n');

    if (mSetLayerIndex == -1)
        error += tr("No set layers found!") + QLatin1Char('\n');

    if (mLayerRuleSets.size() == 0)
        error += tr("No ruleSet layer found!") + QLatin1Char('\n');

    // no need to check for mLayerRuleNotSets.size() == 0 here.
    // these layers are not necessary.

    if (!error.isEmpty()) {
        error = mRulePath + QLatin1Char('\n') + error;
        mError += error;
        return false;
    }

    return true;
}

bool AutoMapper::setupRulesUsedCheck()
{
    TileLayer *setLayer = mMapWork->layerAt(mSetLayerIndex)->asTileLayer();
    QList<Tileset*> tilesetWork = setLayer->usedTilesets().toList();
    foreach (const TileLayer *tilelayer, mLayerRuleSets)
        foreach (Tileset *tileset, tilelayer->usedTilesets())
            if (tileset->findSimilarTileset(tilesetWork) || tilesetWork.contains(tileset))
                return true;

    foreach (const TileLayer *tilelayer, mLayerRuleNotSets)
        foreach (Tileset *tileset, tilelayer->usedTilesets())
            if (tileset->findSimilarTileset(tilesetWork) || tilesetWork.contains(tileset))
                return true;

    return false;
}

bool AutoMapper::setupRuleList()
{
    Q_ASSERT(mRules.isEmpty());
    Q_ASSERT(mLayerRuleRegions);

    mRules = coherentRegions(mLayerRuleRegions->region());

    return true;
}

bool AutoMapper::prepareAutoMap()
{
    mError.clear();
    mWarning.clear();

    if (!setupMissingLayers())
        return false;

    if (!setupCorrectIndexes())
        return false;

    if (!setupTilesets(mMapRules, mMapWork))
        return false;

    return true;
}

bool AutoMapper::setupMissingLayers()
{
    // make sure all needed layers are there:
    foreach (const QString &name, mTouchedLayers) {
        if (mMapWork->indexOfLayer(name) != -1)
            continue;

        const int index =  mMapWork->layerCount();
        TileLayer *tilelayer = new TileLayer(name, 0, 0,
                                             mMapWork->width(),
                                             mMapWork->height());
        mMapDocument->undoStack()->push(
                    new AddLayer(mMapDocument, index, tilelayer));
        mAddedTileLayers.append(name);
    }
    return true;
}

bool AutoMapper::setupCorrectIndexes()
{
    // make sure all indexes of the layer translationtables are correct.
    for (int i = 0; i < mLayerList.size(); ++i) {
        IndexByTileLayer *translationTable = mLayerList.at(i);
        foreach (TileLayer *tileLayerKey, translationTable->keys()) {
            QString name = tileLayerKey->name();
            const int pos = name.indexOf(QLatin1Char('_')) + 1;
            name = name.right(name.length() - pos);

            const int index = translationTable->value(tileLayerKey, -1);
            if (index >= mMapWork->layerCount() || index == -1 ||
                    name != mMapWork->layerAt(index)->name()) {

                int newIndex = mMapWork->indexOfLayer(name);
                Q_ASSERT(newIndex != -1);

                translationTable->insert(tileLayerKey, newIndex);
            }
        }
    }
    // check the set layer as well:
    if (mSetLayerIndex >= mMapWork->layerCount() ||
            mSetLayer != mMapWork->layerAt(mSetLayerIndex)->name()) {

        mSetLayerIndex = mMapWork->indexOfLayer(mSetLayer);

        if (mSetLayerIndex == -1)
            return false;
    }
    return true;
}

/**
 * this cannot just be replaced by MapDocument::unifyTileset(Map),
 * because here mAddedTileset is modified
 */
bool AutoMapper::setupTilesets(Map *src, Map *dst)
{
    QList<Tileset*> existingTilesets = dst->tilesets();

    // Add tilesets that are not yet part of dst map
    foreach (Tileset *tileset, src->tilesets()) {
        if (existingTilesets.contains(tileset))
            continue;

        QUndoStack *undoStack = mMapDocument->undoStack();

        Tileset *replacement = tileset->findSimilarTileset(existingTilesets);
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

        TilesetManager *tilesetManager = TilesetManager::instance();
        tilesetManager->addReference(replacement);
        tilesetManager->removeReference(tileset);
    }
    return true;
}

void AutoMapper::autoMap(QRegion *where)
{
    // first resize the active area
    if (mAutoMappingRadius) {
        QRegion region;
        foreach (const QRect &r, where->rects()) {
            region += r.adjusted(- mAutoMappingRadius,
                                 - mAutoMappingRadius,
                                 + mAutoMappingRadius,
                                 + mAutoMappingRadius);
        }
        *where += region;
    }

    // delete all the relevant area, if the property "DeleteTiles" is set
    if (mDeleteTiles)
        for (int i = 0; i < mLayerList.size(); ++i) {
            IndexByTileLayer *translationTable = mLayerList.at(i);
            foreach (TileLayer *tilelayer, translationTable->keys()) {
                const int index = mLayerList.at(i)->value(tilelayer);
                TileLayer *dstLayer = mMapWork->layerAt(index)->asTileLayer();
                clearRegion(dstLayer, *where);
            }
        }

    // Increase the given region where the next automapper should work.
    // This needs to be done, so you can rely on the order of the rules at all
    // locations
    QRegion ret;
    foreach (const QRect &rect, where->rects())
        foreach (const QRegion &rule, mRules) {
            // at the moment the parallel execution does not work yet
            // TODO: make multithreading available!
            // either by dividing the rules or the region to multiple threads
            ret = ret.united(applyRule(rule, rect));
        }
    *where = where->united(ret);
}

void AutoMapper::clearRegion(TileLayer *dstLayer, const QRegion &where)
{
    TileLayer *setLayer = mMapWork->layerAt(mSetLayerIndex)->asTileLayer();
    QRegion region = where.intersected(dstLayer->bounds());
    foreach (const QRect &rect, region.rects())
        for (int x = rect.left(); x <= rect.right(); ++x)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                if (setLayer->contains(x, y))
                    if (!setLayer->cellAt(x, y).isEmpty())
                        if (dstLayer->contains(x, y))
                            dstLayer->setCell(x, y, Cell());
}

static bool compareLayerTo(const TileLayer *setLayer,
                           const QVector<TileLayer*> &listYes,
                           const QVector<TileLayer*> &listNo,
                           const QRegion &ruleRegion, const QPoint &offset);

QRect AutoMapper::applyRule(const QRegion &rule, const QRect &where)
{
    QRect ret;

    if (mLayerList.isEmpty())
        return ret;

    QRect rbr = rule.boundingRect();

    // Since the rule itself is translated, we need to adjust the borders of the
    // loops. Decrease the size at all sides by one: There must be at least one
    // tile overlap to the rule.
    const int minX = where.left() - rbr.left() - rbr.width() + 1;
    const int minY = where.top() - rbr.top() - rbr.height() + 1;

    const int maxX = where.right() - rbr.left() + rbr.width() - 1;
    const int maxY = where.bottom() - rbr.top() + rbr.height() - 1;
    TileLayer *setLayer = mMapWork->layerAt(mSetLayerIndex)->asTileLayer();

    // In this list of regions it is stored which parts or the map have already
    // been altered by exactly this rule. We store all the altered parts to
    // make sure there are no overlaps of the same rule applied to
    // (neighbouring) places
    QList<QRegion> appliedRegions;
    if (mNoOverlappingRules)
        for (int i = 0; i < mMapWork->layerCount(); i++)
            appliedRegions.append(QRegion());

    for (int y = minY; y <= maxY; ++y)
        for (int x = minX; x <= maxX; ++x)
            if (compareLayerTo(setLayer, mLayerRuleSets,
                               mLayerRuleNotSets, rule, QPoint(x, y))) {
                int r = 0;
                // choose by chance which group of rule_layers should be used:
                if (mLayerList.size() > 1)
                    r = qrand() % mLayerList.size();

                if (!mNoOverlappingRules) {
                    copyMapRegion(rule, QPoint(x, y), mLayerList.at(r));
                    ret = ret.united(rbr.translated(QPoint(x, y)));
                    continue;
                }

                bool missmatch = false;
                IndexByTileLayer *translationTable = mLayerList.at(r);
                QList<TileLayer*> tileLayers = translationTable->keys();

                // check if there are no overlaps within this rule.
                QVector<QRegion> ruleRegionInLayer;
                for (int i = 0; i < tileLayers.size(); ++i) {
                    TileLayer *tilelayer = tileLayers.at(i);
                    ruleRegionInLayer.append(tilelayer->region().intersected(rule));

                    if (appliedRegions.at(i).intersects(
                                ruleRegionInLayer[i].translated(x, y))) {
                        missmatch = true;
                        break;
                    }
                }
                if (!missmatch) {
                    copyMapRegion(rule, QPoint(x, y), mLayerList.at(r));
                    ret = ret.united(rbr.translated(QPoint(x, y)));
                    for (int i = 0; i < translationTable->size(); ++i) {
                        appliedRegions[i] +=
                                ruleRegionInLayer[i].translated(x, y);
                    }
                }
            }

    return ret;
}

/**
 * Returns a list of all cells which can be found within all tile layers
 * within the given region.
 */
static QVector<Cell> cellsInRegion(const QVector<TileLayer*> &list,
                                   const QRegion &r)
{
    QVector<Cell> cells;
    foreach (const TileLayer *tilelayer, list) {
        foreach (const QRect &rect, r.rects()) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                for (int y = rect.top(); y <= rect.bottom(); ++y) {
                    const Cell &cell = tilelayer->cellAt(x, y);
                    if (!cells.contains(cell))
                        cells.append(cell);
                }
            }
        }
    }
    return cells;
}

/**
 * This function is one of the core functions for understanding the
 * automapping.
 * In this function a certain region (of the set layer) is compared to
 * several other layers (ruleSet and ruleNotSet).
 * This comparision will determine if a rule of automapping matches,
 * so if this rule is applied at this region given
 * by a QRegion and Offset given by a QPoint.
 *
 * This compares the tile layer setLayer to several others given
 * in the QList listYes (ruleSet) and OList listNo (ruleNotSet).
 * The tile layer setLayer is examined at QRegion ruleRegion + offset
 * The tile layers within listYes and listNo are examined at QRegion ruleRegion.
 *
 * Basically all matches between setLayer and a layer of listYes are considered
 * good, while all matches between setLayer and listNo are considered bad and
 * lead to canceling the comparison, returning false.
 *
 * The comparison is done for each position within the QRegion ruleRegion.
 * If all positions of the region are considered "good" return true.
 *
 * Now there are several cases to distinguish:
 * - setLayer is 0:
 *      obviously there should be no automapping.
 *      So here no rule should be applied. return false
 * - setLayer is not 0:
 *      - both listYes and listNo are empty:
 *          should not happen, because with that configuration, absolutly
 *          no condition is given.
 *          return false, assuming this is an errornous rule being applied
 *
 *      - both listYes and listNo are not empty:
 *          When comparing a tile at a certain position of tile layer setLayer
 *          to all available tiles in listYes, there must be at least
 *          one layer, in which there is a match of tiles of setLayer and
*           listYes to consider this position good.
 *          In listNo there must not be a match to consider this position
 *          good.
 *          If there are no tiles within all available tiles within all layers
 *          of one list, all tiles in setLayer are considered good,
 *          while inspecting this list.
 *          All available tiles are all tiles within the whole rule region in
 *          all tile layers of the list.
 *
 *      - either of both lists is empty
 *          When comparing a certain position of tile layer setLayer
 *          to all Tiles at the corresponding position this can happen:
 *          A tile of setLayer matches a tile of a layer in the list. Then this
 *          is considered as good, if the layer is from the listYes.
 *          Otherwise it is considered bad.
 *
 *          Exception, when having only the listYes:
 *          if at the examined position there are no tiles within all Layers
 *          of the listYes, all tiles except all used tiles within
 *          the layers of that list are considered good.
 *
 *          This exception was added to have a better functionality
 *          (need of less layers.)
 *          It was not added to the case, when having only listNo layers to
 *          avoid total symmetrie between those lists.
 *
 * If all positions are considered good, return true.
 * return false otherwise.
 *
 * @return bool, if the tile layer matches the given list of layers.
 */
static bool compareLayerTo(const TileLayer *setLayer,
                           const QVector<TileLayer*> &listYes,
                           const QVector<TileLayer*> &listNo,
                           const QRegion &ruleRegion, const QPoint &offset)
{
    if (listYes.isEmpty() && listNo.isEmpty())
        return false;

    QVector<Cell> cells;
    if (listYes.isEmpty())
        cells = cellsInRegion(listNo, ruleRegion);
    if (listNo.isEmpty())
        cells = cellsInRegion(listYes, ruleRegion);

    foreach (const QRect &rect, ruleRegion.rects()) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                // this is only used in the case where only one list has layers
                // it is needed for the exception mentioned above
                bool ruleDefinedListYes = false;

                bool matchListYes = false;
                bool matchListNo  = false;

                if (!setLayer->contains(x + offset.x(), y + offset.y()))
                    return false;

                const Cell &c1 = setLayer->cellAt(x + offset.x(),
                                                  y + offset.y());

                // when there is no tile in setLayer,
                // there should be no rule at all
                if (c1.isEmpty())
                    return false;

                // ruleDefined will be set when there is a tile in at least
                // one layer. if there is a tile in at least one layer, only
                // the given tiles in the different listYes layers are valid.
                // if there is given no tile at all in the listYes layers,
                // consider all tiles valid.

                foreach (const TileLayer *comparedTileLayer, listYes) {

                    if (!comparedTileLayer->contains(x, y))
                        return false;

                    const Cell &c2 = comparedTileLayer->cellAt(x, y);
                    if (!c2.isEmpty())
                        ruleDefinedListYes = true;

                    if (!c2.isEmpty() && c1 == c2)
                        matchListYes = true;
                }
                foreach (const TileLayer *comparedTileLayer, listNo) {

                    if (!comparedTileLayer->contains(x, y))
                        return false;

                    const Cell &c2 = comparedTileLayer->cellAt(x, y);

                    if (!c2.isEmpty() && c1 == c2)
                        matchListNo = true;
                }

                // when there are only layers in the listNo
                // check only if these layers are unmatched
                // no need to check explicitly the exception in this case.
                if (listYes.isEmpty()) {
                    if (matchListNo)
                        return false;
                    else
                        continue;
                }
                // when there are only layers in the listYes
                // check if these layers are matched, or if the exception works
                if (listNo.isEmpty()) {
                    if (matchListYes)
                        continue;
                    if (!ruleDefinedListYes && !cells.contains(c1))
                        continue;
                    return false;
                }

                // there are layers in both lists:
                // no need to consider ruleDefinedListXXX
                if ((matchListYes || !ruleDefinedListYes) && !matchListNo)
                    continue;
                else
                    return false;
            }
        }
    }
    return true;
}

void AutoMapper::copyMapRegion(const QRegion &region, QPoint offset,
                               const IndexByTileLayer *layerTranslation)
{
    for (int i = 0; i < layerTranslation->keys().size(); ++i) {
        TileLayer *from = layerTranslation->keys().at(i);
        TileLayer *to = mMapWork->layerAt(layerTranslation->value(from))->
                                          asTileLayer();
        foreach (const QRect &rect, region.rects()) {
            copyRegion(from,
                       rect.x(), rect.y(),
                       rect.width(), rect.height(),
                       to,
                       rect.x() + offset.x(), rect.y() + offset.y());
        }
    }
}

void AutoMapper::copyRegion(TileLayer *srcLayer, int srcX, int srcY,
                            int width, int height,
                            TileLayer *dstLayer, int dstX, int dstY)
{
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            const Cell &cell = srcLayer->cellAt(srcX + x, srcY + y);
            if (!cell.isEmpty()) {
                // this is without graphics update, it's done afterwards for all
                dstLayer->setCell(dstX + x, dstY + y, cell);
            }
        }
    }
}

void AutoMapper::cleanAll()
{
    cleanTilesets();
    cleanTileLayers();
}

void AutoMapper::cleanTilesets()
{
    foreach (Tileset *tileset, mAddedTilesets) {
        if (mMapWork->isTilesetUsed(tileset))
            continue;

        const int layerIndex = mMapWork->indexOfTileset(tileset);
        if (layerIndex == -1)
            continue;

        QUndoStack *undo = mMapDocument->undoStack();
        undo->push(new RemoveTileset(mMapDocument, layerIndex, tileset));
    }
    mAddedTilesets.clear();
}

void AutoMapper::cleanTileLayers()
{
    foreach (const QString &tilelayerName, mAddedTileLayers) {
        const int layerIndex = mMapWork->indexOfLayer(tilelayerName);
        if (layerIndex == -1)
            continue;

        const TileLayer *tilelayer = mMapWork->layerAt(layerIndex)->asTileLayer();
        if (!tilelayer->isEmpty())
            continue;

        QUndoStack *undo = mMapDocument->undoStack();
        undo->push(new RemoveLayer(mMapDocument, layerIndex));
    }
    mAddedTileLayers.clear();
}

void AutoMapper::cleanUpRulesMap()
{
    cleanTilesets();

    // mMapRules can be empty, when in prepareLoad the very first stages fail.
    if (!mMapRules)
        return;

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->removeReferences(mMapRules->tilesets());

    delete mMapRules;
    mMapRules = 0;

    cleanUpRuleMapLayers();
    mRules.clear();
}

void AutoMapper::cleanUpRuleMapLayers()
{
    cleanTileLayers();

    QList<IndexByTileLayer*>::const_iterator it;
    for (it = mLayerList.constBegin(); it != mLayerList.constEnd(); ++it)
        delete (*it);

    mLayerList.clear();
    // do not delete mLayerRuleRegions, it is owned by the map mapWork, which
    // cares for it
    mLayerRuleRegions = 0;
    mLayerRuleSets.clear();
    mLayerRuleNotSets.clear();
}

