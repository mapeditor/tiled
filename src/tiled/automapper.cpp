/*
 * automapper.cpp
 * Copyright 2010-2012, Stefan Beller, stefanbeller@googlemail.com
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
#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "automappingutils.h"
#include "changeproperties.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "object.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "utils.h"

#include <QDebug>

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

AutoMapper::AutoMapper(MapDocument *workingDocument, Map *rules,
                       const QString &rulePath)
    : mMapDocument(workingDocument)
    , mMapWork(workingDocument ? workingDocument->map() : 0)
    , mMapRules(rules)
    , mLayerInputRegions(0)
    , mLayerOutputRegions(0)
    , mRulePath(rulePath)
    , mDeleteTiles(false)
    , mAutoMappingRadius(0)
    , mNoOverlappingRules(false)
{
    Q_ASSERT(mMapRules);

    if (!setupRuleMapProperties())
        return;

    if (!setupRuleMapTileLayers())
        return;

    if (!setupRuleList())
        return;
}

AutoMapper::~AutoMapper()
{
    cleanUpRulesMap();
}

QSet<QString> AutoMapper::getTouchedTileLayers() const
{
    return mTouchedTileLayers;
}

bool AutoMapper::ruleLayerNameUsed(QString ruleLayerName) const
{
    return mInputRules.names.contains(ruleLayerName);
}

bool AutoMapper::setupRuleMapProperties()
{
    Properties properties = mMapRules->properties();
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
            mWarning += tr("'%1': Property '%2' = '%3' does not make sense. "
                           "Ignoring this property.")
                    .arg(mRulePath, key, value.toString()) + QLatin1Char('\n');
    }
    return true;
}

bool AutoMapper::setupRuleMapTileLayers()
{
    Q_ASSERT(mLayerList.isEmpty());
    Q_ASSERT(mAddedTilesets.isEmpty());
    Q_ASSERT(!mLayerInputRegions);
    Q_ASSERT(!mLayerOutputRegions);
    Q_ASSERT(mInputRules.isEmpty());
    Q_ASSERT(mInputRules.names.isEmpty());
    Q_ASSERT(mInputRules.indexes.isEmpty());

    QString error;

    foreach (Layer *layer, mMapRules->layers()) {
        const QString layerName = layer->name();

        if (layerName.startsWith(QLatin1String("regions"),
                                 Qt::CaseInsensitive)) {
            bool treatAsBoth = layerName.toLower() == QLatin1String("regions");
            if (layerName.endsWith(QLatin1String("input"),
                                 Qt::CaseInsensitive) || treatAsBoth) {
                if (mLayerInputRegions) {
                    error += tr("'input regions layer must not occur more than once.")
                            + QLatin1Char('\n');
                }
                if (layer->isTileLayer()) {
                    mLayerInputRegions = layer->asTileLayer();
                } else {
                    error += tr("regions layer must be tile layer!")
                            + QLatin1Char('\n');
                }
            }
            if (layerName.endsWith(QLatin1String("output"),
                                 Qt::CaseInsensitive) || treatAsBoth) {
                if (mLayerOutputRegions) {
                    error += tr("'output regions layer must not occur more than once.")
                            + QLatin1Char('\n');
                }
                if (layer->isTileLayer()) {
                    mLayerOutputRegions = layer->asTileLayer();
                } else {
                    error += tr("regions layer must be tile layer!")
                            + QLatin1Char('\n');
                }
            }
            continue;
        }

        int nameStartPosition = layerName.indexOf(
                                      QLatin1Char('_')) + 1;
        // name is all characters behind the underscore (excluded)
        QString name = layerName.right(layerName.size() - nameStartPosition);
        // group is all before the underscore (included)
        QString index = layerName.left(nameStartPosition);

        if (index.startsWith(QLatin1String("output"), Qt::CaseInsensitive))
            index.remove(0, 6);
        else if (index.startsWith(QLatin1String("inputnot"), Qt::CaseInsensitive))
            index.remove(0, 8);
        else if (index.startsWith(QLatin1String("input"), Qt::CaseInsensitive))
            index.remove(0, 5);

        // both 'rule' and 'output' layers will require and underscore and
        // rely on the correct position detected of the underscore
        if (nameStartPosition == 0) {
            error += tr("Did you forget an underscore in layer '%1'?").arg(
                        layerName) + QLatin1Char('\n');
            continue;
        }

        if (layerName.startsWith(QLatin1String("input"), Qt::CaseInsensitive)) {

            bool isNotList = layerName.startsWith(QLatin1String("inputnot"),
                                                  Qt::CaseInsensitive);

            if (!layer->isTileLayer()) {
                error += tr("'input' and 'inputnot' layers must be tile layers!")
                        + QLatin1Char('\n');
                continue;
            }

            mInputRules.names.insert(name);

            if (!mInputRules.indexes.contains(index)) {
                mInputRules.indexes.insert(index);
                mInputRules.insert(index, InputIndex());
            }

            if (!mInputRules[index].names.contains(name)) {
                mInputRules[index].names.insert(name);
                mInputRules[index].insert(name, InputIndexName());
            }

            if (isNotList)
                mInputRules[index][name].listNo.append(layer->asTileLayer());
            else
                mInputRules[index][name].listYes.append(layer->asTileLayer());

            continue;
        }

        if (layerName.startsWith(QLatin1String("output"),
                                 Qt::CaseInsensitive)) {
            if (layer->isTileLayer())
                mTouchedTileLayers.insert(name);
            else
                mTouchedObjectGroups.insert(name);

            Layer::Type type = layer->type();
            int layerIndex = mMapWork->indexOfLayer(name, type);

            bool found = false;
            foreach (RuleOutput *translationTable, mLayerList) {
                if (translationTable->index == index) {
                    translationTable->insert(layer, layerIndex);
                    found = true;
                    break;
                }
            }
            if (!found) {
                mLayerList.append(new RuleOutput());
                mLayerList.last()->insert(layer, layerIndex);
                mLayerList.last()->index = index;
            }
            continue;
        }

        error += tr("Layer '%1' is not recognized as a valid layer for Automapping.")
                .arg(layerName) + QLatin1Char('\n');
    }

    if (!mLayerInputRegions)
        error += tr("No input regions layer found!") + QLatin1Char('\n');

    if (!mLayerOutputRegions)
        error += tr("No output regions layer found!") + QLatin1Char('\n');

    if (mInputRules.isEmpty())
        error += tr("No input_<name> layer found!") + QLatin1Char('\n');

    // no need to check for mInputNotRules.size() == 0 here.
    // these layers are not necessary.

    if (!error.isEmpty()) {
        error = mRulePath + QLatin1Char('\n') + error;
        mError += error;
        return false;
    }

    return true;
}

static bool compareRuleRegion(const QRegion &r1, const QRegion &r2)
{
    const QPoint &p1 = r1.boundingRect().topLeft();
    const QPoint &p2 = r2.boundingRect().topLeft();
    return p1.y() < p2.y() || (p1.y() == p2.y() && p1.x() < p2.x());
}

bool AutoMapper::setupRuleList()
{
    Q_ASSERT(mRulesInput.isEmpty());
    Q_ASSERT(mRulesOutput.isEmpty());
    Q_ASSERT(mLayerInputRegions);
    Q_ASSERT(mLayerOutputRegions);

    QList<QRegion> combinedRegions = coherentRegions(
            mLayerInputRegions->region() +
            mLayerOutputRegions->region());

    qSort(combinedRegions.begin(), combinedRegions.end(), compareRuleRegion);

    QList<QRegion> rulesInput = coherentRegions(
            mLayerInputRegions->region());

    QList<QRegion> rulesOutput = coherentRegions(
            mLayerOutputRegions->region());

    for (int i = 0; i < combinedRegions.size(); ++i) {
        mRulesInput.append(QRegion());
        mRulesOutput.append(QRegion());
    }

    foreach(QRegion reg, rulesInput)
        for (int i = 0; i < combinedRegions.size(); ++i) {
            if (reg.intersects(combinedRegions[i])) {
                mRulesInput[i] += reg;
                break;
            }
        }

    foreach(QRegion reg, rulesOutput)
        for (int i = 0; i < combinedRegions.size(); ++i) {
            if (reg.intersects(combinedRegions[i])) {
                mRulesOutput[i] += reg;
                break;
            }
        }

    Q_ASSERT(mRulesInput.size() == mRulesOutput.size());
    for (int i = 0; i < mRulesInput.size(); ++i) {
        const QRegion checkCoherent = mRulesInput.at(i).united(mRulesOutput.at(i));
        Q_ASSERT(coherentRegions(checkCoherent).length() == 1);
    }

    return true;
}

bool AutoMapper::prepareAutoMap()
{
    mError.clear();
    mWarning.clear();

    if (!setupRulesUsedCheck())
        return false;

    if (!setupMissingLayers())
        return false;

    if (!setupCorrectIndexes())
        return false;

    if (!setupTilesets(mMapRules, mMapWork))
        return false;

    return true;
}

bool AutoMapper::setupRulesUsedCheck()
{
    foreach (const QString &index, mInputRules.indexes) {
        foreach (const QString &name, mInputRules[index].names) {
            const InputIndex &ii = mInputRules[index];
            const int i = mMapWork->indexOfLayer(name, Layer::TileLayerType);
            if (i == -1)
                continue;

            const TileLayer *setLayer = mMapWork->layerAt(i)->asTileLayer();
            QList<Tileset*> tilesetWork = setLayer->usedTilesets().toList();

            foreach (const TileLayer *tilelayer, ii[name].listYes)
                foreach (Tileset *tileset, tilelayer->usedTilesets())
                    if (tileset->findSimilarTileset(tilesetWork)
                            || tilesetWork.contains(tileset))
                        return true;

            foreach (const TileLayer *tilelayer, ii[name].listNo)
                foreach (Tileset *tileset, tilelayer->usedTilesets())
                    if (tileset->findSimilarTileset(tilesetWork)
                            || tilesetWork.contains(tileset))
                        return true;
        }
    }
    return false;
}

bool AutoMapper::setupMissingLayers()
{
    // make sure all needed layers are there:
    foreach (const QString &name, mTouchedTileLayers) {
        if (mMapWork->indexOfLayer(name, Layer::TileLayerType) != -1)
            continue;

        const int index =  mMapWork->layerCount();
        TileLayer *tilelayer = new TileLayer(name, 0, 0,
                                             mMapWork->width(),
                                             mMapWork->height());
        mMapDocument->undoStack()->push(
                    new AddLayer(mMapDocument, index, tilelayer));
        mAddedTileLayers.append(name);
    }

    foreach (const QString &name, mTouchedObjectGroups) {
        if (mMapWork->indexOfLayer(name, Layer::ObjectGroupType) != -1)
            continue;

        const int index =  mMapWork->layerCount();
        ObjectGroup *objectGroup = new ObjectGroup(name, 0, 0,
                                                   mMapWork->width(),
                                                   mMapWork->height());
        mMapDocument->undoStack()->push(
                    new AddLayer(mMapDocument, index, objectGroup));
        mAddedTileLayers.append(name);
    }

    return true;
}

bool AutoMapper::setupCorrectIndexes()
{
    // make sure all indexes of the layer translationtables are correct.
    for (int i = 0; i < mLayerList.size(); ++i) {
        RuleOutput *translationTable = mLayerList.at(i);
        foreach (Layer *layerKey, translationTable->keys()) {
            QString name = layerKey->name();
            const int pos = name.indexOf(QLatin1Char('_')) + 1;
            name = name.right(name.length() - pos);

            const int index = translationTable->value(layerKey, -1);
            if (index >= mMapWork->layerCount() || index == -1 ||
                    name != mMapWork->layerAt(index)->name()) {

                int newIndex = mMapWork->indexOfLayer(name, layerKey->type());
                Q_ASSERT(newIndex != -1);

                translationTable->insert(layerKey, newIndex);
            }
        }
    }
    return true;
}

// This cannot just be replaced by MapDocument::unifyTileset(Map),
// because here mAddedTileset is modified.
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
    Q_ASSERT(mRulesInput.size() == mRulesOutput.size());
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
    if (mDeleteTiles) {
        const QRegion setLayersRegion = getSetLayersRegion();
        for (int i = 0; i < mLayerList.size(); ++i) {
            RuleOutput *translationTable = mLayerList.at(i);
            foreach (Layer *layer, translationTable->keys()) {
                const int index = mLayerList.at(i)->value(layer);
                Layer *dstLayer = mMapWork->layerAt(index);
                const QRegion region = setLayersRegion.intersected(*where);
                TileLayer *dstTileLayer = dstLayer->asTileLayer();
                if (dstTileLayer)
                    dstTileLayer->erase(region);
                else
                    eraseRegionObjectGroup(mMapDocument,
                                           dstLayer->asObjectGroup(),
                                           region);
            }
        }
    }

    // Increase the given region where the next automapper should work.
    // This needs to be done, so you can rely on the order of the rules at all
    // locations
    QRegion ret;
    foreach (const QRect &rect, where->rects())
        for (int i = 0; i < mRulesInput.size(); ++i) {
            // at the moment the parallel execution does not work yet
            // TODO: make multithreading available!
            // either by dividing the rules or the region to multiple threads
            ret = ret.united(applyRule(i, rect));
        }
    *where = where->united(ret);
}

const QRegion AutoMapper::getSetLayersRegion()
{
    QRegion result;
    foreach (const QString &name, mInputRules.names) {
        const int index = mMapWork->indexOfLayer(name, Layer::TileLayerType);
        if (index == -1)
            continue;
        TileLayer *setLayer = mMapWork->layerAt(index)->asTileLayer();
        result |= setLayer->region();
    }
    return result;
}

static bool compareLayerTo(const TileLayer *setLayer,
                           const QVector<TileLayer*> &listYes,
                           const QVector<TileLayer*> &listNo,
                           const QRegion &ruleRegion, const QPoint &offset);

QRect AutoMapper::applyRule(const int ruleIndex, const QRect &where)
{
    QRect ret;

    if (mLayerList.isEmpty())
        return ret;

    const QRegion ruleInput = mRulesInput.at(ruleIndex);
    const QRegion ruleOutput = mRulesOutput.at(ruleIndex);
    QRect rbr = ruleInput.boundingRect();

    // Since the rule itself is translated, we need to adjust the borders of the
    // loops. Decrease the size at all sides by one: There must be at least one
    // tile overlap to the rule.
    const int minX = where.left() - rbr.left() - rbr.width() + 1;
    const int minY = where.top() - rbr.top() - rbr.height() + 1;

    const int maxX = where.right() - rbr.left() + rbr.width() - 1;
    const int maxY = where.bottom() - rbr.top() + rbr.height() - 1;

    // In this list of regions it is stored which parts or the map have already
    // been altered by exactly this rule. We store all the altered parts to
    // make sure there are no overlaps of the same rule applied to
    // (neighbouring) places
    QList<QRegion> appliedRegions;
    if (mNoOverlappingRules)
        for (int i = 0; i < mMapWork->layerCount(); i++)
            appliedRegions.append(QRegion());

    for (int y = minY; y <= maxY; ++y)
    for (int x = minX; x <= maxX; ++x) {
        bool anymatch = false;
        foreach (const QString &index, mInputRules.indexes) {
            const InputIndex &ii = mInputRules[index];

            bool allLayerNamesMatch = true;
            foreach (const QString &name, mInputRules[index].names) {
                const int index = mMapWork->indexOfLayer(name,
                                                         Layer::TileLayerType);
                const TileLayer *setLayer = mMapWork->layerAt(index)->asTileLayer();
                allLayerNamesMatch &= compareLayerTo(setLayer,
                                                     ii[name].listYes,
                                                     ii[name].listNo,
                                                     ruleInput, QPoint(x, y));
            }
            if (allLayerNamesMatch) {
                anymatch = true;
                break;
            }
        }

        if (anymatch) {
            int r = 0;
            // choose by chance which group of rule_layers should be used:
            if (mLayerList.size() > 1)
                r = qrand() % mLayerList.size();

            if (!mNoOverlappingRules) {
                copyMapRegion(ruleOutput, QPoint(x, y), mLayerList.at(r));
                ret = ret.united(rbr.translated(QPoint(x, y)));
                continue;
            }

            bool missmatch = false;
            RuleOutput *translationTable = mLayerList.at(r);
            QList<Layer*> layers = translationTable->keys();

            // check if there are no overlaps within this rule.
            QVector<QRegion> ruleRegionInLayer;
            for (int i = 0; i < layers.size(); ++i) {
                Layer *layer = layers.at(i);

                QRegion appliedPlace;
                TileLayer *tileLayer = layer->asTileLayer();
                if (tileLayer)
                    appliedPlace = tileLayer->region();
                else
                    appliedPlace = tileRegionOfObjectGroup(layer->asObjectGroup());

                ruleRegionInLayer.append(appliedPlace.intersected(ruleOutput));
                if (appliedRegions.at(i).intersects(
                            ruleRegionInLayer[i].translated(x, y))) {
                    missmatch = true;
                    break;
                }
            }
            if (missmatch)
                continue;

            copyMapRegion(ruleOutput, QPoint(x, y), mLayerList.at(r));
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
 *          This should not happen, because with that configuration, absolutely
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
 *      - either of both lists are not empty
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
                               const RuleOutput *layerTranslation)
{
    for (int i = 0; i < layerTranslation->keys().size(); ++i) {
        Layer *from = layerTranslation->keys().at(i);
        Layer *to = mMapWork->layerAt(layerTranslation->value(from));
        foreach (const QRect &rect, region.rects()) {
            TileLayer *fromTileLayer = from->asTileLayer();
            ObjectGroup *fromObjectGroup = from->asObjectGroup();
            if (fromTileLayer) {
                TileLayer *toTileLayer = to->asTileLayer();
                Q_ASSERT(toTileLayer); //TODO check this before in prepareAutomap or such!
                copyTileRegion(fromTileLayer, rect.x(), rect.y(),
                               rect.width(), rect.height(),
                               toTileLayer,
                               rect.x() + offset.x(), rect.y() + offset.y());

            } else if (fromObjectGroup) {
                ObjectGroup *toObjectGroup = to->asObjectGroup();
                copyObjectRegion(fromObjectGroup, rect.x(), rect.y(),
                                 rect.width(), rect.height(),
                                 toObjectGroup,
                                 rect.x() + offset.x(), rect.y() + offset.y());
            } else {
                Q_ASSERT(false);
            }
        }
    }
}

void AutoMapper::copyTileRegion(TileLayer *srcLayer, int srcX, int srcY,
                                int width, int height,
                                TileLayer *dstLayer, int dstX, int dstY)
{
    const int startX = qMax(dstX, 0);
    const int startY = qMax(dstY, 0);

    const int endX = qMin(dstX + width, dstLayer->width());
    const int endY = qMin(dstY + height, dstLayer->height());

    const int offsetX = srcX - dstX;
    const int offsetY = srcY - dstY;

    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            const Cell &cell = srcLayer->cellAt(x + offsetX, y + offsetY);
            if (!cell.isEmpty()) {
                // this is without graphics update, it's done afterwards for all
                dstLayer->setCell(x, y, cell);
            }
        }
    }
}

void AutoMapper::copyObjectRegion(ObjectGroup *srcLayer, int srcX, int srcY,
                                 int width, int height,
                                 ObjectGroup *dstLayer, int dstX, int dstY)
{
    QUndoStack *undo = mMapDocument->undoStack();
    const QRect rect = QRect(srcX, srcY, width, height);
    QList<MapObject*> objects = objectsInRegion(srcLayer, rect);

    QList<MapObject*> clones;
    foreach (MapObject *obj, objects) {
        MapObject *clone = obj->clone();
        clones.append(clone);
        clone->setX(clone->x() + dstX - srcX);
        clone->setY(clone->y() + dstY - srcY);
        undo->push(new AddMapObject(mMapDocument, dstLayer, clone));
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
        const int layerIndex = mMapWork->indexOfLayer(tilelayerName,
                                                      Layer::TileLayerType);
        if (layerIndex == -1)
            continue;

        const Layer *layer = mMapWork->layerAt(layerIndex);
        if (!layer->isEmpty())
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
    mRulesInput.clear();
    mRulesOutput.clear();
}

void AutoMapper::cleanUpRuleMapLayers()
{
    cleanTileLayers();

    QList<RuleOutput*>::const_iterator it;
    for (it = mLayerList.constBegin(); it != mLayerList.constEnd(); ++it)
        delete (*it);

    mLayerList.clear();
    // do not delete mLayerRuleRegions, it is owned by the rulesmap
    mLayerInputRegions = 0;
    mLayerOutputRegions = 0;
    mInputRules.clear();
}
