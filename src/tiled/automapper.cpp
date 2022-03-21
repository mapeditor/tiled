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
#include "logginginterface.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "object.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"

#include <QDebug>
#include <QRandomGenerator>

#include <algorithm>

using namespace Tiled;

static int wrap(int value, int bound)
{
    return (value % bound + bound) % bound;
}

template<typename Type, typename Container, typename Pred, typename... Args>
static inline Type &find_or_emplace(Container &container, Pred pred, Args&&... args)
{
    auto it = std::find_if(container.begin(), container.end(), pred);
    if (it != container.end())
        return *it;

    return container.emplace_back(std::forward<Args>(args)...);
}

/*
 * About the order of the methods in this file.
 * The AutoMapper class has 3 bigger public functions, that is
 * prepareAutoMap(), autoMap() and finalizeAutoMap().
 * These three functions make use of lots of different private methods, which
 * are put directly below each of these functions.
 */

AutoMapper::AutoMapper(MapDocument *mapDocument,
                       std::unique_ptr<Map> rulesMap,
                       const QString &rulesMapFileName)
    : mTargetDocument(mapDocument)
    , mTargetMap(mapDocument->map())
    , mRulesMap(std::move(rulesMap))
    , mRulesMapFileName(rulesMapFileName)
{
    Q_ASSERT(mRulesMap);

    if (!setupRuleMapProperties())
        return;

    if (!setupRuleMapLayers())
        return;

    if (!setupRuleList())
        return;
}

AutoMapper::~AutoMapper()
{
    // These should no longer be around
    Q_ASSERT(mAddedLayers.isEmpty());
    Q_ASSERT(mAddedTilesets.isEmpty());
}

bool AutoMapper::ruleLayerNameUsed(const QString &ruleLayerName) const
{
    return mRuleMapSetup.mInputLayerNames.contains(ruleLayerName);
}

bool AutoMapper::setupRuleMapProperties()
{
    QMapIterator<QString, QVariant> it(mRulesMap->properties());
    while (it.hasNext()) {
        it.next();

        const QString &name = it.key();
        const QVariant &value = it.value();

        if (name.compare(QLatin1String("DeleteTiles"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QMetaType::Bool)) {
                mOptions.deleteTiles = value.toBool();
                continue;
            }
        } else if (name.compare(QLatin1String("MatchOutsideMap"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QMetaType::Bool)) {
                mOptions.matchOutsideMap = value.toBool();
                continue;
            }
        } else if (name.compare(QLatin1String("OverflowBorder"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QMetaType::Bool)) {
                mOptions.overflowBorder = value.toBool();
                continue;
            }
        } else if (name.compare(QLatin1String("WrapBorder"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QMetaType::Bool)) {
                mOptions.wrapBorder = value.toBool();
                continue;
            }
        } else if (name.compare(QLatin1String("AutomappingRadius"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QMetaType::Int)) {
                mOptions.autoMappingRadius = value.toInt();
                continue;
            }
        } else if (name.compare(QLatin1String("NoOverlappingRules"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QMetaType::Bool)) {
                mOptions.noOverlappingRules = value.toBool();
                continue;
            }
        }

        addWarning(tr("Ignoring unknown property '%2' = '%3' (rule map '%1')")
                      .arg(mRulesMapFileName,
                           name,
                           value.toString()),
                   SelectCustomProperty { mRulesMapFileName, name, mRulesMap.get() });
    }

    // OverflowBorder and WrapBorder make no sense for infinite maps
    if (mTargetMap->infinite()) {
        mOptions.overflowBorder = false;
        mOptions.wrapBorder = false;

        // Infinite maps have no size, so we always match outside the map
        mOptions.matchOutsideMap = true;
    }

    // Each of the border options imply MatchOutsideMap
    if (mOptions.overflowBorder || mOptions.wrapBorder)
        mOptions.matchOutsideMap = true;

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
            if (value.canConvert(QMetaType::Bool)) {
                inputLayer.strictEmpty = value.toBool();
                continue;
            }
        }

        addWarning(tr("Ignoring unknown property '%2' = '%3' on layer '%4' (rule map '%1')")
                   .arg(mRulesMapFileName,
                        name,
                        value.toString(),
                        inputLayer.tileLayer->name()),
                   SelectCustomProperty { mRulesMapFileName, name, inputLayer.tileLayer });
    }
}

bool AutoMapper::setupRuleMapLayers()
{
    auto &setup = mRuleMapSetup;

    Q_ASSERT(!setup.mLayerRegions);
    Q_ASSERT(!setup.mLayerInputRegions);
    Q_ASSERT(!setup.mLayerOutputRegions);
    Q_ASSERT(setup.mInputSets.empty());
    Q_ASSERT(setup.mOutputSets.empty());
    Q_ASSERT(setup.mInputLayerNames.isEmpty());

    QString error;

    for (Layer *layer : mRulesMap->allLayers()) {
        if (layer->isGroupLayer() || layer->isImageLayer())
            continue;

        const QString &ruleMapLayerName = layer->name();

        // Ignore commented out layers
        if (ruleMapLayerName.startsWith(QLatin1String("//")))
            continue;

        if (ruleMapLayerName.startsWith(QLatin1String("regions"), Qt::CaseInsensitive)) {
            QString layerKind;
            const TileLayer** layerPointer = nullptr;

            if (ruleMapLayerName.compare(QLatin1String("regions"), Qt::CaseInsensitive) == 0) {
                layerKind = QLatin1String("regions");
                layerPointer = &setup.mLayerRegions;
            } else if (ruleMapLayerName.endsWith(QLatin1String("input"), Qt::CaseInsensitive)) {
                layerKind = QLatin1String("regions_input");
                layerPointer = &setup.mLayerInputRegions;
            } else if (ruleMapLayerName.endsWith(QLatin1String("output"), Qt::CaseInsensitive)) {
                layerKind = QLatin1String("regions_output");
                layerPointer = &setup.mLayerOutputRegions;
            } else {
                addWarning(tr("Layer '%1' is not recognized as a valid layer for Automapping.").arg(ruleMapLayerName),
                           SelectLayer { layer });
                continue;
            }

            if (*layerPointer) {
                error += tr("'%1' layer must not occur more than once.").arg(layerKind);
                error += QLatin1Char('\n');
            }

            if (TileLayer *tileLayer = layer->asTileLayer()) {
                *layerPointer = tileLayer;
            } else {
                error += tr("'regions_*' layers must be tile layers.");
                error += QLatin1Char('\n');
            }

            continue;
        }

        const int layerNameStartPosition = ruleMapLayerName.indexOf(QLatin1Char('_')) + 1;

        // both 'rule' and 'output' layers will require and underscore and
        // rely on the correct position detected of the underscore
        if (layerNameStartPosition == 0) {
            error += tr("Did you forget an underscore in layer '%1'?").arg(ruleMapLayerName);
            error += QLatin1Char('\n');
            continue;
        }

        const QString layerName = ruleMapLayerName.mid(layerNameStartPosition);  // all characters behind the underscore (excluded)
        QString setName = ruleMapLayerName.left(layerNameStartPosition);         // all before the underscore (included)

        if (setName.startsWith(QLatin1String("output"), Qt::CaseInsensitive))
            setName.remove(0, 6);
        else if (setName.startsWith(QLatin1String("inputnot"), Qt::CaseInsensitive))
            setName.remove(0, 8);
        else if (setName.startsWith(QLatin1String("input"), Qt::CaseInsensitive))
            setName.remove(0, 5);

        if (ruleMapLayerName.startsWith(QLatin1String("input"), Qt::CaseInsensitive)) {
            const TileLayer *tileLayer = layer->asTileLayer();

            if (!tileLayer) {
                error += tr("'input_*' and 'inputnot_*' layers must be tile layers.");
                error += QLatin1Char('\n');
                continue;
            }

            setup.mInputLayerNames.insert(layerName);

            InputLayer inputLayer;
            inputLayer.tileLayer = tileLayer;
            setupInputLayerProperties(inputLayer);

            InputSet &inputSet = find_or_emplace<InputSet>(setup.mInputSets, [&setName] (const InputSet &set) {
                return set.name == setName;
            }, setName);

            InputConditions &conditions = find_or_emplace<InputConditions>(inputSet.layers, [&layerName] (const InputConditions &conditions) {
                return conditions.layerName == layerName;
            }, layerName);

            const bool isNotList = ruleMapLayerName.startsWith(QLatin1String("inputnot"), Qt::CaseInsensitive);
            if (isNotList)
                conditions.listNo.append(inputLayer);
            else
                conditions.listYes.append(inputLayer);

            continue;
        }

        if (ruleMapLayerName.startsWith(QLatin1String("output"), Qt::CaseInsensitive)) {
            if (layer->isTileLayer())
                mTouchedTileLayers.insert(layerName, nullptr);
            else if (layer->isObjectGroup())
                mTouchedObjectGroups.insert(layerName, nullptr);

            OutputSet &outputSet = find_or_emplace<OutputSet>(setup.mOutputSets, [&setName] (const OutputSet &set) {
                return set.name == setName;
            }, setName);

            outputSet.layers.insert(layer, layerName);

            continue;
        }

        addWarning(tr("Layer '%1' is not recognized as a valid layer for Automapping.").arg(ruleMapLayerName),
                   SelectLayer { layer });
    }

    if (setup.mInputSets.empty())
        error += tr("No input_<name> or inputnot_<name> layer found!") + QLatin1Char('\n');

    if (setup.mOutputSets.empty())
        error += tr("No output_<name> layer found!") + QLatin1Char('\n');

    // Make sure the input layers are always matched in the same order, which
    // significantly speeds up the matching logic.
    for (InputSet &set : setup.mInputSets) {
        std::sort(set.layers.begin(), set.layers.end(),
                  [] (const InputConditions &a, const InputConditions &b) { return a.layerName < b.layerName; });
    }

    if (!error.isEmpty()) {
        error = mRulesMapFileName + QLatin1Char('\n') + error;
        mError += error;
        return false;
    }

    return true;
}

static bool compareRuleRegion(const QRegion &r1, const QRegion &r2)
{
    const QPoint p1 = r1.boundingRect().topLeft();
    const QPoint p2 = r2.boundingRect().topLeft();
    return p1.y() < p2.y() || (p1.y() == p2.y() && p1.x() < p2.x());
}

bool AutoMapper::setupRuleList()
{
    const auto &setup = mRuleMapSetup;

    Q_ASSERT(mRuleRegions.isEmpty());

    QRegion regionInput;
    QRegion regionOutput;

    if (setup.mLayerRegions)
        regionInput = regionOutput = setup.mLayerRegions->region();
    if (setup.mLayerInputRegions)
        regionInput |= setup.mLayerInputRegions->region();
    if (setup.mLayerOutputRegions)
        regionOutput |= setup.mLayerOutputRegions->region();

    // When no input regions have been defined at all, derive them from the
    // "input" and "inputnot" layers.
    if (!setup.mLayerRegions && !setup.mLayerInputRegions) {
        for (const InputSet &inputIndex : qAsConst(mRuleMapSetup.mInputSets)) {
            for (const InputConditions &conditions : inputIndex.layers) {
                for (const InputLayer &inputLayer : conditions.listNo)
                    regionInput |= inputLayer.tileLayer->region();
                for (const InputLayer &inputLayer : conditions.listYes)
                    regionInput |= inputLayer.tileLayer->region();
            }
        }
    }

    // When no output regions have been defined at all, derive them from the
    // "output" layers.
    if (!setup.mLayerRegions && !setup.mLayerOutputRegions) {
        for (const OutputSet &ruleOutput : qAsConst(mRuleMapSetup.mOutputSets)) {
            std::for_each(ruleOutput.layers.keyBegin(),
                          ruleOutput.layers.keyEnd(),
                          [&] (const Layer *layer) {
                if (layer->isTileLayer())
                    regionOutput |= static_cast<const TileLayer*>(layer)->region();
            });
        }
    }

    QVector<QRegion> combinedRegions = coherentRegions(regionInput + regionOutput);

    // Combined regions are sorted, in order to have a deterministic order in
    // which the rules are applied.
    std::sort(combinedRegions.begin(), combinedRegions.end(), compareRuleRegion);

    // Then, they are split up in input and output region for each rule.
    mRuleRegions.resize(combinedRegions.size());

    for (int i = 0; i < combinedRegions.size(); ++i) {
        mRuleRegions[i].input = combinedRegions[i] & regionInput;
        mRuleRegions[i].output = combinedRegions[i] & regionOutput;
    }

#ifndef QT_NO_DEBUG
    for (const RuleRegion &ruleRegion : mRuleRegions) {
        const QRegion checkCoherent = ruleRegion.input.united(ruleRegion.output);
        Q_ASSERT(coherentRegions(checkCoherent).size() == 1);
    }
#endif

    return true;
}

void AutoMapper::prepareAutoMap()
{
    mWarning.clear();
    mError.clear();

    setupWorkMapLayers();
    setupTilesets();
}

/**
 * Makes sure all needed output layers are present in the working map.
 */
void AutoMapper::setupWorkMapLayers()
{
    QUndoStack *undoStack = mTargetDocument->undoStack();

    // Set up pointers to touched tile layers (output layers in mTargetMap).
    // They are created when they are not present.
    QMutableHashIterator<QString, TileLayer*> itTileLayers(mTouchedTileLayers);
    while (itTileLayers.hasNext()) {
        itTileLayers.next();

        const QString &name = itTileLayers.key();
        auto tileLayer = static_cast<TileLayer*>(mTargetMap->findLayer(name, Layer::TileLayerType));

        if (!tileLayer) {
            const int index =  mTargetMap->layerCount();
            tileLayer = new TileLayer(name, 0, 0,
                                      mTargetMap->width(),
                                      mTargetMap->height());
            undoStack->push(new AddLayer(mTargetDocument, index, tileLayer, nullptr));
            mAddedLayers.append(tileLayer);
        }

        itTileLayers.setValue(tileLayer);
    }

    // Set up pointers to touched object layers (output layers in mTargetMap).
    // They are created when they are not present.
    QMutableHashIterator<QString, ObjectGroup*> itObjectGroups(mTouchedObjectGroups);
    while (itObjectGroups.hasNext()) {
        itObjectGroups.next();

        const QString &name = itObjectGroups.key();
        auto objectGroup = static_cast<ObjectGroup*>(mTargetMap->findLayer(name, Layer::ObjectGroupType));

        if (!objectGroup) {
            const int index =  mTargetMap->layerCount();
            objectGroup = new ObjectGroup(name, 0, 0);
            undoStack->push(new AddLayer(mTargetDocument, index, objectGroup, nullptr));
            mAddedLayers.append(objectGroup);
        }

        itObjectGroups.setValue(objectGroup);
    }

    // Set up pointers to "set" layers (input layers in mTargetMap). They don't
    // need to be created if not present.
    for (const QString &name : qAsConst(mRuleMapSetup.mInputLayerNames))
        if (auto tileLayer = static_cast<TileLayer*>(mTargetMap->findLayer(name, Layer::TileLayerType)))
            mSetLayers.insert(name, tileLayer);

    for (InputSet &inputIndex : mRuleMapSetup.mInputSets)
        for (InputConditions &conditions : inputIndex.layers)
            conditions.layer = mSetLayers.value(conditions.layerName);
}

/**
 * Makes sure the tilesets present in the rules map are present in the
 * target map.
 */
void AutoMapper::setupTilesets()
{
    Q_ASSERT(mAddedTilesets.isEmpty());

    mTargetDocument->unifyTilesets(*mRulesMap, mAddedTilesets);

    for (const SharedTileset &tileset : qAsConst(mAddedTilesets))
        mTargetDocument->undoStack()->push(new AddTileset(mTargetDocument, tileset));
}

void AutoMapper::autoMap(const QRegion &where, QRegion *appliedRegion)
{
    QRegion applyRegion;

    // first resize the active area if applicable
    if (mOptions.autoMappingRadius) {
        for (const QRect &r : where) {
            applyRegion |= r.adjusted(- mOptions.autoMappingRadius,
                                      - mOptions.autoMappingRadius,
                                      + mOptions.autoMappingRadius,
                                      + mOptions.autoMappingRadius);
        }
    } else {
        applyRegion = where;
    }

    // Delete all the relevant area, if the property "DeleteTiles" is set
    if (mOptions.deleteTiles) {
        // In principle we erase the entire applyRegion, excluding areas where
        // none of the input layers have any contents.
        QRegion inputLayersRegion;
        for (const QString &name : qAsConst(mRuleMapSetup.mInputLayerNames)) {
            if (const TileLayer *setLayer = mSetLayers.value(name))
                inputLayersRegion |= setLayer->region();
        }

        const QRegion regionToErase = inputLayersRegion.intersected(applyRegion);
        for (const OutputSet &ruleOutput : qAsConst(mRuleMapSetup.mOutputSets)) {
            QHashIterator<const Layer*, QString> it(ruleOutput.layers);
            while (it.hasNext()) {
                it.next();

                const QString &name = it.value();

                switch (it.key()->layerType()) {
                case Layer::TileLayerType:
                    mTouchedTileLayers.value(name)->erase(regionToErase);
                    break;
                case Layer::ObjectGroupType:
                    eraseRegionObjectGroup(mTargetDocument,
                                           mTouchedObjectGroups.value(name),
                                           regionToErase);
                    break;
                case Layer::ImageLayerType:
                case Layer::GroupLayerType:
                    Q_UNREACHABLE();
                    break;
                }
            }
        }
    }

    for (const RuleRegion &ruleRegion : mRuleRegions) {
        // at the moment the parallel execution does not work yet
        // TODO: make multithreading available!
        // either by dividing the rules or the region to multiple threads
        applyRule(ruleRegion, applyRegion, appliedRegion);
    }
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
        for (const QRect &rect : r) {
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
                                   const QPoint offset,
                                   const AutoMapper::Options &options)
{
    const auto &listYes = conditions.listYes;
    const auto &listNo = conditions.listNo;

    QVarLengthArray<Cell, 8> cells;

    for (const QRect &rect : ruleRegion) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                int xd = x + offset.x();
                int yd = y + offset.y();

                // Those two options are guaranteed to be false if the map is infinite,
                // so no "invalid" width/height accessing here.
                if (options.wrapBorder) {
                    xd = wrap(xd, setLayer.width());
                    yd = wrap(yd, setLayer.height());
                } else if (options.overflowBorder) {
                    xd = qBound(0, xd, setLayer.width() - 1);
                    yd = qBound(0, yd, setLayer.height() - 1);
                }

                const Cell &setCell = setLayer.cellAt(xd, yd);

                // First check listNo. If any tile matches there, we can
                // immediately know there is no match.
                for (const InputLayer &inputNotLayer : listNo) {
                    const Cell &noCell = inputNotLayer.tileLayer->cellAt(x, y);
                    if ((inputNotLayer.strictEmpty || !noCell.isEmpty()) && setCell == noCell)
                        return false;
                }

                // ruleDefinedListYes will be set when there is a tile in at
                // least one layer. In this case, only the given tiles in the
                // different listYes layers are valid. Otherwise, consider all
                // tiles not used elsewhere in the input as valid.
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
                    if (listNo.isEmpty()) {
                        if (cells.isEmpty())
                            collectCellsInRegion(listYes, ruleRegion, cells);
                        if (cells.contains(setCell))
                            return false;
                    }
                } else if (!matchListYes) {
                    return false;
                }
            }
        }
    }

    return true;
}

void AutoMapper::applyRule(const RuleRegion &ruleRegion,
                           const QRegion &applyRegion,
                           QRegion *appliedRegion)
{
    Q_ASSERT(!mRuleMapSetup.mOutputSets.empty());

    const QRegion &ruleInputRegion = ruleRegion.input;
    const QRegion &ruleOutputRegion = ruleRegion.output;

    // These regions store which parts or the map have already been altered by
    // exactly this rule. We store all the altered parts to make sure there are
    // no overlaps of the same rule applied to (neighbouring) places.
    QHash<const Layer*, QRegion> appliedRegions;

    const TileLayer dummy(QString(), 0, 0, mTargetMap->width(), mTargetMap->height());

    QRandomGenerator *randomGenerator = QRandomGenerator::global();

    auto applyAt = [&] (int x, int y) {
        const bool match = std::any_of(mRuleMapSetup.mInputSets.cbegin(),
                                       mRuleMapSetup.mInputSets.cend(),
                                       [&] (const InputSet &inputIndex) {

            return std::all_of(inputIndex.layers.cbegin(),
                               inputIndex.layers.cend(),
                               [&] (const InputConditions &conditions) {

                const TileLayer &setLayer = conditions.layer ? *conditions.layer : dummy;
                return layerMatchesConditions(setLayer, conditions, ruleInputRegion, QPoint(x, y), mOptions);
            });
        });

        if (!match)
            return;

        // choose by chance which group of rule_layers should be used:
        const int r = randomGenerator->generate() % mRuleMapSetup.mOutputSets.size();
        const OutputSet &ruleOutput = mRuleMapSetup.mOutputSets.at(r);

        if (mOptions.noOverlappingRules) {
            // check if there are no overlaps within this rule.
            QHash<const Layer*, QRegion> ruleRegionInLayer;

            const bool overlap = std::any_of(ruleOutput.layers.keyBegin(),
                                             ruleOutput.layers.keyEnd(),
                                             [&] (const Layer *layer) {
                QRegion outputLayerRegion;

                // TODO: Very slow to re-calculate the entire region for
                // each rule output layer here, each time a rule has a match.
                switch (layer->layerType()) {
                case Layer::TileLayerType:
                    outputLayerRegion = static_cast<const TileLayer*>(layer)->region();
                    break;
                case Layer::ObjectGroupType:
                    outputLayerRegion = tileRegionOfObjectGroup(static_cast<const ObjectGroup*>(layer));
                    break;
                case Layer::ImageLayerType:
                case Layer::GroupLayerType:
                    Q_UNREACHABLE();
                    return false;
                }

                outputLayerRegion &= ruleOutputRegion;
                outputLayerRegion.translate(x, y);

                ruleRegionInLayer[layer] = outputLayerRegion;

                return appliedRegions[layer].intersects(outputLayerRegion);
            });

            if (overlap)
                return;

            // Remember the newly applied region
            std::for_each(ruleOutput.layers.keyBegin(),
                          ruleOutput.layers.keyEnd(),
                          [&] (const Layer *layer) {
                appliedRegions[layer] |= ruleRegionInLayer[layer];
            });
        }

        copyMapRegion(ruleOutputRegion, QPoint(x, y), ruleOutput);

        if (appliedRegion)
            *appliedRegion |= ruleOutputRegion.translated(x, y);
    };

    const QRect inputBounds = ruleInputRegion.boundingRect();

    // This is really the rule size - 1, since when applying the rule we will
    // keep at least one tile overlap with the apply region.
    const int ruleWidth = inputBounds.right() - inputBounds.left();
    const int ruleHeight = inputBounds.bottom() - inputBounds.top();

    QRegion ruleApplyRegion;

    for (const QRect &rect : applyRegion) {
        // Expand each rect, making sure that there is at least one tile
        // overlap with the rule at all sides.
        ruleApplyRegion |= rect.adjusted(-ruleWidth, -ruleHeight, 0, 0);
    }

    // When we're not matching a rule outside the map, we reduce the region in
    // in which it is applied accordingly.
    if (!mOptions.matchOutsideMap) {
        ruleApplyRegion &= QRect(0, 0,
                                 mTargetMap->width() - ruleWidth,
                                 mTargetMap->height() - ruleHeight);
    }

    // Translate the region to adjust to the position of the rule.
    ruleApplyRegion.translate(-inputBounds.topLeft());

    for (const QRect &rect : ruleApplyRegion) {
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                applyAt(x, y);
    }
}

void AutoMapper::copyMapRegion(const QRegion &region, QPoint offset,
                               const OutputSet &ruleOutput)
{
    for (auto it = ruleOutput.layers.begin(), end = ruleOutput.layers.end(); it != end; ++it) {
        const Layer *from = it.key();
        const QString &targetName = it.value();

        Layer *to = nullptr;

        switch (from->layerType()) {
        case Layer::TileLayerType:
            to = mTouchedTileLayers.value(targetName);
            break;
        case Layer::ObjectGroupType:
            to = mTouchedObjectGroups.value(targetName);
            break;
        case Layer::ImageLayerType:
        case Layer::GroupLayerType:
            Q_UNREACHABLE();
            break;
        }
        Q_ASSERT(to);

        for (const QRect &rect : region) {
            switch (from->layerType()) {
            case Layer::TileLayerType: {
                auto fromTileLayer = static_cast<const TileLayer*>(from);
                auto toTileLayer = static_cast<TileLayer*>(to);
                copyTileRegion(fromTileLayer, rect, toTileLayer,
                               rect.x() + offset.x(), rect.y() + offset.y());
                break;
            }
            case Layer::ObjectGroupType: {
                auto fromObjectGroup = static_cast<const ObjectGroup*>(from);
                auto toObjectGroup = static_cast<ObjectGroup*>(to);
                copyObjectRegion(fromObjectGroup, rect, toObjectGroup,
                                 rect.x() + offset.x(), rect.y() + offset.y());
                break;
            }
            case Layer::ImageLayerType:
            case Layer::GroupLayerType:
                Q_UNREACHABLE();
                break;
            }
        }

        // Copy any custom properties set on the output layer
        if (!from->properties().isEmpty()) {
            Properties mergedProperties = to->properties();
            mergeProperties(mergedProperties, from->properties());

            if (mergedProperties != to->properties()) {
                QUndoStack *undoStack = mTargetDocument->undoStack();
                undoStack->push(new ChangeProperties(mTargetDocument, QString(),
                                                     to, mergedProperties));
            }
        }
    }
}

void AutoMapper::copyTileRegion(const TileLayer *srcLayer, QRect rect,
                                TileLayer *dstLayer, int dstX, int dstY)
{
    int startX = dstX;
    int startY = dstY;

    int endX = dstX + rect.width();
    int endY = dstY + rect.height();

    int dwidth = dstLayer->width();
    int dheight = dstLayer->height();

    if (!mOptions.wrapBorder && !mTargetMap->infinite()) {
        startX = qMax(0, startX);
        startY = qMax(0, startY);
        endX = qMin(dwidth, endX);
        endY = qMin(dheight, endY);
    }

    const int offsetX = rect.x() - dstX;
    const int offsetY = rect.y() - dstY;

    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            const Cell &cell = srcLayer->cellAt(x + offsetX, y + offsetY);
            if (!cell.isEmpty()) {
                // this is without graphics update, it's done afterwards for all
                int xd = x;
                int yd = y;

                // WrapBorder is only true on finite maps
                if (mOptions.wrapBorder) {
                    xd = wrap(xd, dwidth);
                    yd = wrap(yd, dheight);
                }
                dstLayer->setCell(xd, yd, cell);
            }
        }
    }
}

void AutoMapper::copyObjectRegion(const ObjectGroup *srcLayer, const QRectF &rect,
                                  ObjectGroup *dstLayer, int dstX, int dstY)
{
    const QRectF pixelRect = mTargetDocument->renderer()->tileToPixelCoords(rect);
    const QList<MapObject*> objects = objectsInRegion(srcLayer, pixelRect.toAlignedRect());

    QPointF pixelOffset = mTargetDocument->renderer()->tileToPixelCoords(dstX, dstY);
    pixelOffset -= pixelRect.topLeft();

    QVector<AddMapObjects::Entry> objectsToAdd;
    objectsToAdd.reserve(objects.size());

    for (MapObject *obj : objects) {
        MapObject *clone = obj->clone();
        clone->resetId();
        clone->setX(clone->x() + pixelOffset.x());
        clone->setY(clone->y() + pixelOffset.y());
        objectsToAdd.append(AddMapObjects::Entry { clone, dstLayer });
    }

    mTargetDocument->undoStack()->push(new AddMapObjects(mTargetDocument, objectsToAdd));
}

void AutoMapper::finalizeAutoMap()
{
    cleanTilesets();
    cleanEmptyLayers();
}

void AutoMapper::cleanTilesets()
{
    QUndoStack *undoStack = mTargetDocument->undoStack();

    for (const SharedTileset &tileset : qAsConst(mAddedTilesets)) {
        if (mTargetMap->isTilesetUsed(tileset.data()))
            continue;

        const int index = mTargetMap->indexOfTileset(tileset);
        if (index == -1)
            continue;

        undoStack->push(new RemoveTileset(mTargetDocument, index));
    }

    mAddedTilesets.clear();
}

void AutoMapper::cleanEmptyLayers()
{
    QUndoStack *undoStack = mTargetDocument->undoStack();

    for (Layer *layer : qAsConst(mAddedLayers)) {
        if (!layer->isEmpty())
            continue;

        const int index = layer->siblingIndex();
        GroupLayer *parentLayer = layer->parentLayer();

        undoStack->push(new RemoveLayer(mTargetDocument, index, parentLayer));
    }

    mAddedLayers.clear();
}

void AutoMapper::addWarning(const QString &message, std::function<void ()> callback)
{
    WARNING(message, std::move(callback));
    mWarning += message;
    mWarning += QLatin1Char('\n');
}

#include "moc_automapper.cpp"
