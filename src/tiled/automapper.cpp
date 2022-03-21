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

using namespace Tiled;

static int wrap(int value, int bound)
{
    return (value % bound + bound) % bound;
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
    , mTargetMap(mapDocument ? mapDocument->map() : nullptr)
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
    return mInputLayers.names.contains(ruleLayerName);
}

bool AutoMapper::setupRuleMapProperties()
{
    // By default, only infinite maps match rules outside of their boundaries
    mOptions.matchOutsideMap = mTargetMap->infinite();

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
    Q_ASSERT(mOutputLayerGroups.isEmpty());
    Q_ASSERT(mAddedTilesets.isEmpty());
    Q_ASSERT(!mLayerInputRegions);
    Q_ASSERT(!mLayerOutputRegions);
    Q_ASSERT(mInputLayers.isEmpty());
    Q_ASSERT(mInputLayers.names.isEmpty());

    QString error;

    for (Layer *layer : mRulesMap->allLayers()) {
        if (layer->isGroupLayer() || layer->isImageLayer())
            continue;

        const QString &layerName = layer->name();

        // Ignore commented out layers
        if (layerName.startsWith(QLatin1String("//")))
            continue;

        if (layerName.startsWith(QLatin1String("regions"), Qt::CaseInsensitive)) {
            QString layerKind;
            const TileLayer** layerPointer = nullptr;

            if (layerName.compare(QLatin1String("regions"), Qt::CaseInsensitive) == 0) {
                layerKind = QLatin1String("regions");
                layerPointer = &mLayerRegions;
            } else if (layerName.endsWith(QLatin1String("input"), Qt::CaseInsensitive)) {
                layerKind = QLatin1String("regions_input");
                layerPointer = &mLayerInputRegions;
            } else if (layerName.endsWith(QLatin1String("output"), Qt::CaseInsensitive)) {
                layerKind = QLatin1String("regions_output");
                layerPointer = &mLayerOutputRegions;
            } else {
                addWarning(tr("Layer '%1' is not recognized as a valid layer for Automapping.").arg(layerName),
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

        const int nameStartPosition = layerName.indexOf(QLatin1Char('_')) + 1;

        // both 'rule' and 'output' layers will require and underscore and
        // rely on the correct position detected of the underscore
        if (nameStartPosition == 0) {
            error += tr("Did you forget an underscore in layer '%1'?").arg(layerName);
            error += QLatin1Char('\n');
            continue;
        }

        const QString name = layerName.mid(nameStartPosition);  // all characters behind the underscore (excluded)
        QString index = layerName.left(nameStartPosition);      // all before the underscore (included)

        if (index.startsWith(QLatin1String("output"), Qt::CaseInsensitive))
            index.remove(0, 6);
        else if (index.startsWith(QLatin1String("inputnot"), Qt::CaseInsensitive))
            index.remove(0, 8);
        else if (index.startsWith(QLatin1String("input"), Qt::CaseInsensitive))
            index.remove(0, 5);

        if (layerName.startsWith(QLatin1String("input"), Qt::CaseInsensitive)) {
            const TileLayer *tileLayer = layer->asTileLayer();

            if (!tileLayer) {
                error += tr("'input_*' and 'inputnot_*' layers must be tile layers.");
                error += QLatin1Char('\n');
                continue;
            }

            const bool isNotList = layerName.startsWith(QLatin1String("inputnot"), Qt::CaseInsensitive);

            mInputLayers.names.insert(name);

            InputLayer inputLayer;
            inputLayer.tileLayer = tileLayer;
            setupInputLayerProperties(inputLayer);

            InputConditions &conditions = mInputLayers[index][name];
            if (isNotList)
                conditions.listNo.append(inputLayer);
            else
                conditions.listYes.append(inputLayer);

            continue;
        }

        if (layerName.startsWith(QLatin1String("output"), Qt::CaseInsensitive)) {
            if (layer->isTileLayer())
                mTouchedTileLayers.insert(name, nullptr);
            else if (layer->isObjectGroup())
                mTouchedObjectGroups.insert(name, nullptr);

            bool found = false;
            for (RuleOutput &translationTable : mOutputLayerGroups) {
                if (translationTable.index == index) {
                    translationTable.insert(layer, name);
                    found = true;
                    break;
                }
            }
            if (!found) {
                mOutputLayerGroups.append(RuleOutput());
                mOutputLayerGroups.last().insert(layer, name);
                mOutputLayerGroups.last().index = index;
            }
            continue;
        }

        addWarning(tr("Layer '%1' is not recognized as a valid layer for Automapping.").arg(layerName),
                   SelectLayer { layer });
    }

    if (!mLayerRegions && !mLayerInputRegions)
        error += tr("No 'regions' or 'regions_input' layer found.") + QLatin1Char('\n');

    if (!mLayerRegions && !mLayerOutputRegions)
        error += tr("No 'regions' or 'regions_output' layer found.") + QLatin1Char('\n');

    if (mInputLayers.isEmpty())
        error += tr("No input_<name> layer found!") + QLatin1Char('\n');

    if (mTouchedTileLayers.isEmpty() && mTouchedObjectGroups.isEmpty())
        error += tr("No output_<name> layer found!") + QLatin1Char('\n');

    // no need to check for mInputNotRules.size() == 0 here.
    // these layers are not necessary.

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
    Q_ASSERT(mRuleRegions.isEmpty());
    Q_ASSERT(mLayerRegions || mLayerInputRegions);
    Q_ASSERT(mLayerRegions || mLayerOutputRegions);

    QRegion regionInput;
    QRegion regionOutput;

    if (mLayerRegions)
        regionInput = regionOutput = mLayerRegions->region();
    if (mLayerInputRegions)
        regionInput |= mLayerInputRegions->region();
    if (mLayerOutputRegions)
        regionOutput |= mLayerOutputRegions->region();

    QVector<QRegion> combinedRegions = coherentRegions(regionInput + regionOutput);
    const QVector<QRegion> rulesInput = coherentRegions(regionInput);
    const QVector<QRegion> rulesOutput = coherentRegions(regionOutput);

    // Combined regions are sorted, in order to have a deterministic order in
    // which the rules are applied.
    std::sort(combinedRegions.begin(), combinedRegions.end(), compareRuleRegion);

    mRuleRegions.resize(combinedRegions.size());

    for (const QRegion &region : rulesInput) {
        for (int i = 0; i < combinedRegions.size(); ++i) {
            if (region.intersects(combinedRegions[i])) {
                mRuleRegions[i].input += region;
                break;
            }
        }
    }

    for (const QRegion &region : rulesOutput) {
        for (int i = 0; i < combinedRegions.size(); ++i) {
            if (region.intersects(combinedRegions[i])) {
                mRuleRegions[i].output += region;
                break;
            }
        }
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
    QMutableMapIterator<QString, TileLayer*> itTileLayers(mTouchedTileLayers);
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
    QMutableMapIterator<QString, ObjectGroup*> itObjectGroups(mTouchedObjectGroups);
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
    for (const QString &name : qAsConst(mInputLayers.names))
        if (auto tileLayer = static_cast<TileLayer*>(mTargetMap->findLayer(name, Layer::TileLayerType)))
            mSetLayers.insert(name, tileLayer);
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

void AutoMapper::autoMap(QRegion *where)
{
    if (mOutputLayerGroups.isEmpty())
        return;

    // first resize the active area
    if (mOptions.autoMappingRadius) {
        QRegion region;
        for (const QRect &r : *where) {
            region += r.adjusted(- mOptions.autoMappingRadius,
                                 - mOptions.autoMappingRadius,
                                 + mOptions.autoMappingRadius,
                                 + mOptions.autoMappingRadius);
        }
        where->swap(region);
    }

    // Delete all the relevant area, if the property "DeleteTiles" is set
    if (mOptions.deleteTiles) {
        const QRegion setLayersRegion = computeSetLayersRegion();
        const QRegion regionToErase = setLayersRegion.intersected(*where);
        for (const RuleOutput &translationTable : qAsConst(mOutputLayerGroups)) {
            QMapIterator<const Layer*, QString> it(translationTable);
            while (it.hasNext()) {
                it.next();

                const QString &name = it.value();

                switch (it.key()->layerType()) {
                case Tiled::Layer::TileLayerType:
                    mTouchedTileLayers.value(name)->erase(regionToErase);
                    break;
                case Tiled::Layer::ObjectGroupType:
                    eraseRegionObjectGroup(mTargetDocument,
                                           mTouchedObjectGroups.value(name),
                                           regionToErase);
                    break;
                case Tiled::Layer::ImageLayerType:
                case Tiled::Layer::GroupLayerType:
                    Q_UNREACHABLE();
                    break;
                }
            }
        }
    }

    // Increase the given region where the next automapper should work.
    // This needs to be done, so you can rely on the order of the rules at all
    // locations
    QRegion ret;
    for (const QRect &rect : *where) {
        for (const RuleRegion &ruleRegion : mRuleRegions) {
            // at the moment the parallel execution does not work yet
            // TODO: make multithreading available!
            // either by dividing the rules or the region to multiple threads
            ret |= applyRule(ruleRegion, rect);
        }
    }
    *where = where->united(ret);
}

QRegion AutoMapper::computeSetLayersRegion() const
{
    QRegion result;
    for (const QString &name : mInputLayers.names) {
        if (const TileLayer *setLayer = mSetLayers.value(name))
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

                if (!options.matchOutsideMap && !setLayer.contains(xd, yd))
                    return false;

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

QRect AutoMapper::applyRule(const RuleRegion &ruleRegion, const QRect &where)
{
    Q_ASSERT(!mOutputLayerGroups.isEmpty());

    QRect ret;

    const QRegion &ruleInputRegion = ruleRegion.input;
    const QRegion &ruleOutputRegion = ruleRegion.output;
    const QRect inputBounds = ruleInputRegion.boundingRect();

    // Since the rule itself is translated, we need to adjust the borders of the
    // loops. Decrease the size at all sides by one: There must be at least one
    // tile overlap to the rule.
    const int minX = where.left() - inputBounds.left() - inputBounds.width() + 1;
    const int minY = where.top() - inputBounds.top() - inputBounds.height() + 1;

    const int maxX = where.right() - inputBounds.left() + inputBounds.width() - 1;
    const int maxY = where.bottom() - inputBounds.top() + inputBounds.height() - 1;

    // These regions store which parts or the map have already been altered by
    // exactly this rule. We store all the altered parts to make sure there are
    // no overlaps of the same rule applied to (neighbouring) places.
    QMap<const Layer*, QRegion> appliedRegions;

    const TileLayer dummy(QString(), 0, 0, mTargetMap->width(), mTargetMap->height());

    QRandomGenerator *randomGenerator = QRandomGenerator::global();

    for (int y = minY; y <= maxY; ++y)
    for (int x = minX; x <= maxX; ++x) {
        bool anyMatch = false;

        for (const InputIndex &inputIndex : qAsConst(mInputLayers)) {
            bool allLayerNamesMatch = true;

            QMapIterator<QString, InputConditions> inputIndexIterator(inputIndex);
            while (inputIndexIterator.hasNext()) {
                inputIndexIterator.next();

                const QString &name = inputIndexIterator.key();
                const InputConditions &conditions = inputIndexIterator.value();

                const TileLayer &setLayer = *mSetLayers.value(name, &dummy);

                if (!layerMatchesConditions(setLayer, conditions, ruleInputRegion, QPoint(x, y), mOptions)) {
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
            const int r = randomGenerator->generate() % mOutputLayerGroups.size();
            const RuleOutput &ruleOutput = mOutputLayerGroups.at(r);

            if (mOptions.noOverlappingRules) {
                bool overlap = false;

                // check if there are no overlaps within this rule.
                QMap<const Layer*, QRegion> ruleRegionInLayer;

                QMapIterator<const Layer*, QString> it(ruleOutput);
                while (it.hasNext()) {
                    const Layer *layer = it.next().key();

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
                        continue;
                    }

                    outputLayerRegion &= ruleOutputRegion;
                    outputLayerRegion.translate(x, y);

                    ruleRegionInLayer[layer] = outputLayerRegion;

                    if (appliedRegions[layer].intersects(outputLayerRegion)) {
                        overlap = true;
                        break;
                    }
                }

                if (overlap)
                    continue;

                // Remember the newly applied region
                it.toFront();
                while (it.hasNext()) {
                    const Layer *layer = it.next().key();
                    appliedRegions[layer] |= ruleRegionInLayer[layer];
                }
            }

            copyMapRegion(ruleOutputRegion, QPoint(x, y), ruleOutput);
            ret |= inputBounds.translated(QPoint(x, y));
        }
    }

    return ret;
}

void AutoMapper::copyMapRegion(const QRegion &region, QPoint offset,
                               const RuleOutput &layerTranslation)
{
    for (auto it = layerTranslation.begin(), end = layerTranslation.end(); it != end; ++it) {
        const Layer *from = it.key();
        const QString &targetName = it.value();

        Layer *to = nullptr;

        switch (from->layerType()) {
        case Tiled::Layer::TileLayerType:
            to = mTouchedTileLayers.value(targetName);
            break;
        case Tiled::Layer::ObjectGroupType:
            to = mTouchedObjectGroups.value(targetName);
            break;
        case Tiled::Layer::ImageLayerType:
        case Tiled::Layer::GroupLayerType:
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
