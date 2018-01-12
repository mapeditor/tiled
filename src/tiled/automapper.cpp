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
#include "geometry.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "object.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetmanager.h"

#include <QDebug>

#include "qtcompat_p.h"

using namespace Tiled;
using namespace Tiled::Internal;

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
    , mMapWork(workingDocument ? workingDocument->map() : nullptr)
    , mMapRules(rules)
    , mLayerInputRegions(nullptr)
    , mLayerOutputRegions(nullptr)
    , mRulePath(rulePath)
    , mDeleteTiles(false)
    , mAutoMappingRadius(0)
    , mNoOverlappingRules(false)
{
    Q_ASSERT(mMapRules);

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(rules->tilesets());

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
    QMapIterator<QString, QVariant> it(mMapRules->properties());
    while (it.hasNext()) {
        it.next();

        const QString &name = it.key();
        const QVariant &value = it.value();

        if (name.compare(QLatin1String("deletetiles"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QVariant::Bool)) {
                mDeleteTiles = value.toBool();
                continue;
            }
        } else if (name.compare(QLatin1String("automappingradius"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QVariant::Int)) {
                mAutoMappingRadius = value.toInt();
                continue;
            }
        } else if (name.compare(QLatin1String("nooverlappingrules"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QVariant::Bool)) {
                mNoOverlappingRules = value.toBool();
                continue;
            }
        }

        mWarning += tr("'%1': Property '%2' = '%3' does not make sense. "
                       "Ignoring this property.")
                .arg(mRulePath, name, value.toString()) + QLatin1Char('\n');
    }
    return true;
}

void AutoMapper::setupInputLayerProperties(InputLayer &inputLayer)
{
    inputLayer.strictEmpty = false;

    QMapIterator<QString, QVariant> it(inputLayer.tileLayer->properties());
    while (it.hasNext()) {
        it.next();

        const QString &name = it.key();
        const QVariant &value = it.value();

        if (name.compare(QLatin1String("strictempty"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QVariant::Bool)) {
                inputLayer.strictEmpty = value.toBool();
                continue;
            }
        }

        mWarning += tr("'%1': Property '%2' = '%3' on layer '%4' does not make sense. "
                       "Ignoring this property.")
                .arg(mRulePath, name, value.toString(), inputLayer.tileLayer->name()) + QLatin1Char('\n');
    }
}

bool AutoMapper::setupRuleMapTileLayers()
{
    Q_ASSERT(mLayerList.isEmpty());
    Q_ASSERT(mAddedTilesets.isEmpty());
    Q_ASSERT(!mLayerInputRegions);
    Q_ASSERT(!mLayerOutputRegions);
    Q_ASSERT(mInputRules.isEmpty());
    Q_ASSERT(mInputRules.names.isEmpty());

    QString error;

    for (Layer *layer : mMapRules->layers()) {
        const QString layerName = layer->name();

        if (layerName.startsWith(QLatin1String("regions"), Qt::CaseInsensitive)) {
            bool inputAndOutput = layerName.compare(QLatin1String("regions"), Qt::CaseInsensitive) == 0;

            if (inputAndOutput || layerName.endsWith(QLatin1String("input"), Qt::CaseInsensitive)) {
                if (mLayerInputRegions) {
                    error += tr("'regions_input' layer must not occur more than once.");
                    error += QLatin1Char('\n');
                }
                if (TileLayer *tileLayer = layer->asTileLayer()) {
                    mLayerInputRegions = tileLayer;
                } else {
                    error += tr("'regions_*' layers must be tile layers.");
                    error += QLatin1Char('\n');
                }
            }

            if (inputAndOutput || layerName.endsWith(QLatin1String("output"), Qt::CaseInsensitive)) {
                if (mLayerOutputRegions) {
                    error += tr("'regions_output' layer must not occur more than once.");
                    error += QLatin1Char('\n');
                }
                if (TileLayer *tileLayer = layer->asTileLayer()) {
                    mLayerOutputRegions = tileLayer;
                } else {
                    error += tr("'regions_*' layers must be tile layers.");
                    error += QLatin1Char('\n');
                }
            }

            continue;
        }

        int nameStartPosition = layerName.indexOf(QLatin1Char('_')) + 1;

        // both 'rule' and 'output' layers will require and underscore and
        // rely on the correct position detected of the underscore
        if (nameStartPosition == 0) {
            error += tr("Did you forget an underscore in layer '%1'?").arg(layerName);
            error += QLatin1Char('\n');
            continue;
        }

        QString name = layerName.mid(nameStartPosition);    // all characters behind the underscore (excluded)
        QString index = layerName.left(nameStartPosition);  // all before the underscore (included)

        if (index.startsWith(QLatin1String("output"), Qt::CaseInsensitive))
            index.remove(0, 6);
        else if (index.startsWith(QLatin1String("inputnot"), Qt::CaseInsensitive))
            index.remove(0, 8);
        else if (index.startsWith(QLatin1String("input"), Qt::CaseInsensitive))
            index.remove(0, 5);

        if (layerName.startsWith(QLatin1String("input"), Qt::CaseInsensitive)) {
            bool isNotList = layerName.startsWith(QLatin1String("inputnot"), Qt::CaseInsensitive);

            TileLayer *tileLayer = layer->asTileLayer();

            if (!tileLayer) {
                error += tr("'input_*' and 'inputnot_*' layers must be tile layers.");
                error += QLatin1Char('\n');
                continue;
            }

            mInputRules.names.insert(name);

            if (!mInputRules.contains(index))
                mInputRules.insert(index, InputIndex());

            if (!mInputRules[index].contains(name))
                mInputRules[index].insert(name, InputConditions());

            InputLayer inputLayer;
            inputLayer.tileLayer = tileLayer;
            setupInputLayerProperties(inputLayer);

            InputConditions &conditions = mInputRules[index][name];
            if (isNotList)
                conditions.listNo.append(inputLayer);
            else
                conditions.listYes.append(inputLayer);

            continue;
        }

        if (layerName.startsWith(QLatin1String("output"), Qt::CaseInsensitive)) {
            if (layer->isTileLayer())
                mTouchedTileLayers.insert(name);
            else if (layer->isObjectGroup())
                mTouchedObjectGroups.insert(name);

            Layer::TypeFlag type = layer->layerType();
            int layerIndex = mMapWork->indexOfLayer(name, type);

            bool found = false;
            for (RuleOutput &translationTable : mLayerList) {
                if (translationTable.index == index) {
                    translationTable.insert(layer, layerIndex);
                    found = true;
                    break;
                }
            }
            if (!found) {
                mLayerList.append(RuleOutput());
                mLayerList.last().insert(layer, layerIndex);
                mLayerList.last().index = index;
            }
            continue;
        }

        error += tr("Layer '%1' is not recognized as a valid layer for Automapping.")
                .arg(layerName) + QLatin1Char('\n');
    }

    if (!mLayerInputRegions)
        error += tr("No 'regions' or 'regions_input' layer found.") + QLatin1Char('\n');

    if (!mLayerOutputRegions)
        error += tr("No 'regions' or 'regions_output' layer found.") + QLatin1Char('\n');

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

    QVector<QRegion> combinedRegions = coherentRegions(mLayerInputRegions->region() +
                                                       mLayerOutputRegions->region());

    qSort(combinedRegions.begin(), combinedRegions.end(), compareRuleRegion);

    const QVector<QRegion> rulesInput = coherentRegions(mLayerInputRegions->region());
    const QVector<QRegion> rulesOutput = coherentRegions(mLayerOutputRegions->region());

    mRulesInput.resize(combinedRegions.size());
    mRulesOutput.resize(combinedRegions.size());

    for (const QRegion &reg : rulesInput) {
        for (int i = 0; i < combinedRegions.size(); ++i) {
            if (reg.intersects(combinedRegions[i])) {
                mRulesInput[i] += reg;
                break;
            }
        }
    }

    for (const QRegion &reg : rulesOutput) {
        for (int i = 0; i < combinedRegions.size(); ++i) {
            if (reg.intersects(combinedRegions[i])) {
                mRulesOutput[i] += reg;
                break;
            }
        }
    }

    Q_ASSERT(mRulesInput.size() == mRulesOutput.size());
    for (int i = 0; i < mRulesInput.size(); ++i) {
        const QRegion checkCoherent = mRulesInput.at(i).united(mRulesOutput.at(i));
        Q_ASSERT(coherentRegions(checkCoherent).size() == 1);
    }

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

    if (!setupTilesets())
        return false;

    return true;
}

bool AutoMapper::setupMissingLayers()
{
    QUndoStack *undoStack = mMapDocument->undoStack();

    // make sure all needed layers are there:
    foreach (const QString &name, mTouchedTileLayers) {
        if (mMapWork->indexOfLayer(name, Layer::TileLayerType) != -1)
            continue;

        const int index =  mMapWork->layerCount();
        TileLayer *tileLayer = new TileLayer(name, 0, 0,
                                             mMapWork->width(),
                                             mMapWork->height());
        undoStack->push(new AddLayer(mMapDocument, index, tileLayer, nullptr));
        mAddedLayers.append(tileLayer);
    }

    foreach (const QString &name, mTouchedObjectGroups) {
        if (mMapWork->indexOfLayer(name, Layer::ObjectGroupType) != -1)
            continue;

        const int index =  mMapWork->layerCount();
        ObjectGroup *objectGroup = new ObjectGroup(name, 0, 0);
        undoStack->push(new AddLayer(mMapDocument, index, objectGroup, nullptr));
        mAddedLayers.append(objectGroup);
    }

    return true;
}

bool AutoMapper::setupCorrectIndexes()
{
    // make sure all indexes of the layer translation tables are correct.
    for (RuleOutput &translationTable : mLayerList) {
        foreach (Layer *layerKey, translationTable.keys()) {
            QString name = layerKey->name();
            const int pos = name.indexOf(QLatin1Char('_')) + 1;
            name = name.right(name.length() - pos);

            const int index = translationTable.value(layerKey, -1);
            if (index >= mMapWork->layerCount() || index == -1 ||
                    name != mMapWork->layerAt(index)->name()) {

                int newIndex = mMapWork->indexOfLayer(name, layerKey->layerType());
                Q_ASSERT(newIndex != -1);

                translationTable.insert(layerKey, newIndex);
            }
        }
    }
    return true;
}

bool AutoMapper::setupTilesets()
{
    Q_ASSERT(mAddedTilesets.isEmpty());

    mMapDocument->unifyTilesets(mMapRules, mAddedTilesets);

    for (const SharedTileset &tileset : qAsConst(mAddedTilesets))
        mMapDocument->undoStack()->push(new AddTileset(mMapDocument, tileset));

    return true;
}

void AutoMapper::autoMap(QRegion *where)
{
    Q_ASSERT(mRulesInput.size() == mRulesOutput.size());
    // first resize the active area
    if (mAutoMappingRadius) {
        QRegion region;
#if QT_VERSION < 0x050800
        foreach (const QRect &r, where->rects()) {
#else
        for (const QRect &r : *where) {
#endif
            region += r.adjusted(- mAutoMappingRadius,
                                 - mAutoMappingRadius,
                                 + mAutoMappingRadius,
                                 + mAutoMappingRadius);
        }
        where->swap(region);
    }

    // delete all the relevant area, if the property "DeleteTiles" is set
    if (mDeleteTiles) {
        const QRegion setLayersRegion = computeSetLayersRegion();
        for (const RuleOutput &translationTable : mLayerList) {
            for (const int index : translationTable) {
                Layer *dstLayer = mMapWork->layerAt(index);
                const QRegion region = setLayersRegion.intersected(*where);
                TileLayer *dstTileLayer = dstLayer->asTileLayer();
                if (dstTileLayer) {
                    dstTileLayer->erase(region);
                } else {
                    eraseRegionObjectGroup(mMapDocument,
                                           dstLayer->asObjectGroup(),
                                           region);
                }
            }
        }
    }

    // Increase the given region where the next automapper should work.
    // This needs to be done, so you can rely on the order of the rules at all
    // locations
    QRegion ret;
#if QT_VERSION < 0x050800
    foreach (const QRect &rect, where->rects()) {
#else
    for (const QRect &rect : *where) {
#endif
        for (int i = 0; i < mRulesInput.size(); ++i) {
            // at the moment the parallel execution does not work yet
            // TODO: make multithreading available!
            // either by dividing the rules or the region to multiple threads
            ret = ret.united(applyRule(i, rect));
        }
    }
    *where = where->united(ret);
}

QRegion AutoMapper::computeSetLayersRegion() const
{
    QRegion result;
    for (const QString &name : qAsConst(mInputRules.names)) {
        const int index = mMapWork->indexOfLayer(name, Layer::TileLayerType);
        if (index == -1)
            continue;
        TileLayer *setLayer = mMapWork->layerAt(index)->asTileLayer();
        result |= setLayer->region();
    }
    return result;
}

/**
 * Fills \a cells with the list of all cells which can be found within all
 * tile layers within the given region.
 */
static void collectCellsInRegion(const QVector<InputLayer> &list,
                                 const QRegion &r,
                                 QVarLengthArray<Cell, 8> &cells)
{
    for (const InputLayer &inputLayer : list) {
#if QT_VERSION < 0x050800
        foreach (const QRect &rect, r.rects()) {
#else
        for (const QRect &rect : r) {
#endif
            for (int x = rect.left(); x <= rect.right(); ++x) {
                for (int y = rect.top(); y <= rect.bottom(); ++y) {
                    const Cell &cell = inputLayer.tileLayer->cellAt(x, y);
                    if (!cells.contains(cell))
                        cells.append(cell);
                }
            }
        }
    }
}

/**
 * This function is one of the core functions for understanding the
 * automapping.
 * In this function a certain region (of the set layer) is compared to
 * several other layers (ruleSet and ruleNotSet).
 * This comparison will determine if a rule of automapping matches,
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
 *  - both listYes and listNo are empty:
 *      This should not happen, because with that configuration, absolutely
 *      no condition is given.
 *      return false, assuming this is an errornous rule being applied
 *
 *  - both listYes and listNo are not empty:
 *      When comparing a tile at a certain position of tile layer setLayer
 *      to all available tiles in listYes, there must be at least
 *      one layer, in which there is a match of tiles of setLayer and
 *      listYes to consider this position good.
 *      In listNo there must not be a match to consider this position
 *      good.
 *      If there are no tiles within all available tiles within all layers
 *      of one list, all tiles in setLayer are considered good,
 *      while inspecting this list.
 *      All available tiles are all tiles within the whole rule region in
 *      all tile layers of the list.
 *
 *  - either of both lists are not empty
 *      When comparing a certain position of tile layer setLayer
 *      to all Tiles at the corresponding position this can happen:
 *      A tile of setLayer matches a tile of a layer in the list. Then this
 *      is considered as good, if the layer is from the listYes.
 *      Otherwise it is considered bad.
 *
 *      Exception, when having only the listYes:
 *      if at the examined position there are no tiles within all Layers
 *      of the listYes, all tiles except all used tiles within
 *      the layers of that list are considered good.
 *
 *      This exception was added to have a better functionality
 *      (need of less layers.)
 *      It was not added to the case, when having only listNo layers to
 *      avoid total symmetry between those lists.
 *      It can be turned off by setting the StrictEmpty property on the input
 *      layer.
 *
 * If all positions are considered good, return true.
 * return false otherwise.
 *
 * @return bool, if the tile layer matches the given list of layers.
 */
static bool layerMatchesConditions(const TileLayer &setLayer,
                                   const InputConditions &conditions,
                                   const QRegion &ruleRegion,
                                   const QPoint &offset)
{
    const auto &listYes = conditions.listYes;
    const auto &listNo = conditions.listNo;
    if (listYes.isEmpty() && listNo.isEmpty())
        return false;

    QVarLengthArray<Cell, 8> cells;
    if (listNo.isEmpty())
        collectCellsInRegion(listYes, ruleRegion, cells);

#if QT_VERSION < 0x050800
    foreach (const QRect &rect, ruleRegion.rects()) {
#else
    for (const QRect &rect : ruleRegion) {
#endif
        for (int x = rect.left(); x <= rect.right(); ++x) {
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                const Cell &setCell = setLayer.cellAt(x + offset.x(),
                                                      y + offset.y());

                // First check listNo. If any tile matches there, we can
                // immediately know there is no match.
                for (const InputLayer &inputNotLayer : listNo) {
                    const Cell &noCell = inputNotLayer.tileLayer->cellAt(x, y);
                    if ((inputNotLayer.strictEmpty || !noCell.isEmpty()) && setCell == noCell)
                        return false;
                }

                // ruleDefinedListYes will be set when there is a tile in at
                // least one layer. If there is a tile in at least one layer,
                // only the given tiles in the different listYes layers are
                // valid. Otherwise, consider all tiles not used elsewhere in
                // the input as valid.
                bool ruleDefinedListYes = false;
                bool matchListYes = false;

                for (const InputLayer &inputLayer : listYes) {
                    const Cell &yesCell = inputLayer.tileLayer->cellAt(x, y);
                    if (inputLayer.strictEmpty || !yesCell.isEmpty()) {
                        ruleDefinedListYes = true;
                        if (setCell == yesCell) {
                            matchListYes = true;
                            break;
                        }
                    }
                }

                if (!ruleDefinedListYes) {
                    // if there were only layers in the listYes, check the exception
                    if (listNo.isEmpty() && cells.contains(setCell))
                        return false;
                } else if (!matchListYes) {
                    return false;
                }
            }
        }
    }

    return true;
}

QRect AutoMapper::applyRule(int ruleIndex, const QRect &where)
{
    QRect ret;

    if (mLayerList.isEmpty())
        return ret;

    const QRegion &ruleInputRegion = mRulesInput.at(ruleIndex);
    const QRegion &ruleOutputRegion = mRulesOutput.at(ruleIndex);
    const QRect rbr = ruleInputRegion.boundingRect();

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
    QVector<QRegion> appliedRegions;
    if (mNoOverlappingRules)
        appliedRegions.resize(mMapWork->layerCount());

    const TileLayer dummy(QString(), 0, 0, 0, 0);

    for (int y = minY; y <= maxY; ++y)
    for (int x = minX; x <= maxX; ++x) {
        bool anyMatch = false;

        for (const InputIndex &inputIndex : qAsConst(mInputRules)) {
            bool allLayerNamesMatch = true;

            QMapIterator<QString, InputConditions> inputIndexIterator(inputIndex);
            while (inputIndexIterator.hasNext()) {
                inputIndexIterator.next();

                const QString &name = inputIndexIterator.key();
                const InputConditions &conditions = inputIndexIterator.value();

                const int i = mMapWork->indexOfLayer(name, Layer::TileLayerType);
                const TileLayer &setLayer = (i >= 0) ? *mMapWork->layerAt(i)->asTileLayer() : dummy;

                if (!layerMatchesConditions(setLayer, conditions, ruleInputRegion, QPoint(x, y))) {
                    allLayerNamesMatch = false;
                    break;
                }
            }

            if (allLayerNamesMatch) {
                anyMatch = true;
                break;
            }
        }

        if (anyMatch) {
            // choose by chance which group of rule_layers should be used:
            const int r = qrand() % mLayerList.size();
            const RuleOutput &translationTable = mLayerList.at(r);

            if (mNoOverlappingRules) {
                bool overlap = false;
                const QList<Layer*> layers = translationTable.keys();

                // check if there are no overlaps within this rule.
                QVector<QRegion> ruleRegionInLayer;
                for (int i = 0; i < layers.size(); ++i) {
                    Layer *layer = layers.at(i);

                    QRegion appliedPlace;

                    if (TileLayer *tileLayer = layer->asTileLayer())
                        appliedPlace = tileLayer->region();
                    else if (ObjectGroup *objectGroup = layer->asObjectGroup())
                        appliedPlace = tileRegionOfObjectGroup(objectGroup);
                    else
                        continue;

                    ruleRegionInLayer.append(appliedPlace.intersected(ruleOutputRegion));

                    if (appliedRegions.at(i).intersects(ruleRegionInLayer.at(i).translated(x, y))) {
                        overlap = true;
                        break;
                    }
                }

                if (overlap)
                    continue;

                for (int i = 0; i < translationTable.size(); ++i)
                    appliedRegions[i] += ruleRegionInLayer.at(i).translated(x, y);
            }

            copyMapRegion(ruleOutputRegion, QPoint(x, y), translationTable);
            ret = ret.united(rbr.translated(QPoint(x, y)));
        }
    }

    return ret;
}

void AutoMapper::copyMapRegion(const QRegion &region, QPoint offset,
                               const RuleOutput &layerTranslation)
{
    for (auto it = layerTranslation.begin(), end = layerTranslation.end(); it != end; ++it) {
        Layer *from = it.key();
        Layer *to = mMapWork->layerAt(it.value());

#if QT_VERSION < 0x050800
        foreach (const QRect &rect, region.rects()) {
#else
        for (const QRect &rect : region) {
#endif
            if (TileLayer *fromTileLayer = from->asTileLayer()) {
                TileLayer *toTileLayer = to->asTileLayer();
                Q_ASSERT(toTileLayer); //TODO check this before in prepareAutomap or such!
                copyTileRegion(fromTileLayer, rect.x(), rect.y(),
                               rect.width(), rect.height(),
                               toTileLayer,
                               rect.x() + offset.x(), rect.y() + offset.y());

            } else if (ObjectGroup *fromObjectGroup = from->asObjectGroup()) {
                ObjectGroup *toObjectGroup = to->asObjectGroup();
                copyObjectRegion(fromObjectGroup, rect.x(), rect.y(),
                                 rect.width(), rect.height(),
                                 toObjectGroup,
                                 rect.x() + offset.x(), rect.y() + offset.y());
            } else {
                Q_ASSERT(false);
            }
        }

        // Copy any custom properties set on the output layer
        if (!from->properties().isEmpty()) {
            Properties mergedProperties = to->properties();
            mergedProperties.merge(from->properties());

            if (mergedProperties != to->properties()) {
                QUndoStack *undoStack = mMapDocument->undoStack();
                undoStack->push(new ChangeProperties(mMapDocument, QString(),
                                                     to, mergedProperties));
            }
        }
    }
}

void AutoMapper::copyTileRegion(const TileLayer *srcLayer, int srcX, int srcY,
                                int width, int height,
                                TileLayer *dstLayer, int dstX, int dstY)
{
    int startX = dstX;
    int startY = dstY;

    int endX = dstX + width;
    int endY = dstY + height;

    if (!mMapWork->infinite()) {
        startX = qMax(0, startX);
        startY = qMax(0, startY);
        endX = qMin(dstLayer->width(), endX);
        endY = qMin(dstLayer->height(), endY);
    }

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

void AutoMapper::copyObjectRegion(const ObjectGroup *srcLayer, int srcX, int srcY,
                                  int width, int height,
                                  ObjectGroup *dstLayer, int dstX, int dstY)
{
    QUndoStack *undo = mMapDocument->undoStack();
    const QRectF rect = QRectF(srcX, srcY, width, height);
    const QRectF pixelRect = mMapDocument->renderer()->tileToPixelCoords(rect);
    const QList<MapObject*> objects = objectsInRegion(srcLayer, pixelRect.toAlignedRect());

    QPointF pixelOffset = mMapDocument->renderer()->tileToPixelCoords(dstX, dstY);
    pixelOffset -= pixelRect.topLeft();

    for (MapObject *obj : objects) {
        MapObject *clone = obj->clone();
        clone->resetId();
        clone->setX(clone->x() + pixelOffset.x());
        clone->setY(clone->y() + pixelOffset.y());
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
    QUndoStack *undoStack = mMapDocument->undoStack();

    for (const SharedTileset &tileset : qAsConst(mAddedTilesets)) {
        if (mMapWork->isTilesetUsed(tileset.data()))
            continue;

        const int index = mMapWork->indexOfTileset(tileset);
        if (index == -1)
            continue;

        undoStack->push(new RemoveTileset(mMapDocument, index));
    }

    mAddedTilesets.clear();
}

void AutoMapper::cleanTileLayers()
{
    QUndoStack *undoStack = mMapDocument->undoStack();

    for (Layer *layer : qAsConst(mAddedLayers)) {
        if (!layer->isEmpty())
            continue;

        const int index = layer->siblingIndex();
        GroupLayer *parentLayer = layer->parentLayer();

        undoStack->push(new RemoveLayer(mMapDocument, index, parentLayer));
    }

    mAddedLayers.clear();
}

void AutoMapper::cleanUpRulesMap()
{
    cleanTilesets();

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->removeReferences(mMapRules->tilesets());

    delete mMapRules;
    mMapRules = nullptr;

    cleanUpRuleMapLayers();
    mRulesInput.clear();
    mRulesOutput.clear();
}

void AutoMapper::cleanUpRuleMapLayers()
{
    cleanTileLayers();

    mLayerList.clear();

    // do not delete these, they are owned by the rules map
    mLayerInputRegions = nullptr;
    mLayerOutputRegions = nullptr;
    mInputRules.clear();
}
