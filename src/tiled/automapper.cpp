/*
 * automapper.cpp
 * Copyright 2010-2016, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2016-2022, Thorbjørn Lindijer <bjorn@lindeijer.nl>
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

#include "automappingutils.h"
#include "containerhelpers.h"
#include "geometry.h"
#include "logginginterface.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "tile.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QtConcurrent>

#include <algorithm>
#include <random>

namespace Tiled {

static int wrap(int value, int bound)
{
    return (value % bound + bound) % bound;
}

static const Cell &getWrappedCell(int x, int y, const TileLayer &tileLayer)
{
    return tileLayer.cellAt(wrap(x, tileLayer.width()),
                            wrap(y, tileLayer.height()));
}

static const Cell &getBoundCell(int x, int y, const TileLayer &tileLayer)
{
    return tileLayer.cellAt(qBound(0, x, tileLayer.width() - 1),
                            qBound(0, y, tileLayer.height() - 1));
}

static const Cell &getCell(int x, int y, const TileLayer &tileLayer)
{
    return tileLayer.cellAt(x, y);
}

static double randomDouble()
{
    thread_local std::mt19937 engine(std::random_device{}());
    std::uniform_real_distribution<double> dist(0, 1);
    return dist(engine);
}

template<typename Type, typename Container, typename Pred, typename... Args>
static inline Type &find_or_emplace(Container &container, Pred pred, Args&&... args)
{
    auto it = std::find_if(container.begin(), container.end(), pred);
    if (it != container.end())
        return *it;

    return container.emplace_back(std::forward<Args>(args)...);
}

enum class MatchType {
    Unknown,
    Tile,
    Empty,
    NonEmpty,
    Other,
    Negate,
    Ignore,
};

static MatchType matchType(const Tile *tile)
{
    if (!tile)
        return MatchType::Unknown;

    const QString matchType = tile->resolvedProperty(QStringLiteral("MatchType")).toString();
    if (matchType == QLatin1String("Empty"))
        return MatchType::Empty;
    if (matchType == QLatin1String("NonEmpty"))
        return MatchType::NonEmpty;
    if (matchType == QLatin1String("Other"))
        return MatchType::Other;
    if (matchType == QLatin1String("Negate"))
        return MatchType::Negate;
    if (matchType == QLatin1String("Ignore"))
        return MatchType::Ignore;

    return MatchType::Tile;
}

/**
 * The compile context enables re-using temporarily allocated memory while
 * compiling the rules.
 */
struct CompileContext
{
    QVector<MatchCell> anyOf;
    QVector<MatchCell> noneOf;
    QVector<MatchCell> inputCells;
};

struct ApplyContext
{
    ApplyContext(QRegion *appliedRegion)
        : appliedRegion(appliedRegion)
    {}

    // These regions store which parts of the map have already been altered by
    // exactly this rule. We store all the altered parts to make sure there are
    // no overlaps of the same rule applied to (neighbouring) places.
    QHash<const Layer*, QRegion> appliedRegions;

    QRegion *appliedRegion;
};


AutoMappingContext::AutoMappingContext(MapDocument *mapDocument)
    : targetDocument(mapDocument)
    , targetMap(targetDocument->map())
{
}


AutoMapper::AutoMapper(std::unique_ptr<Map> rulesMap, const QRegularExpression &mapNameFilter)
    : mRulesMap(std::move(rulesMap))
    , mRulesMapRenderer(MapRenderer::create(mRulesMap.get()))
    , mMapNameFilter(mapNameFilter)
{
    setupRuleMapProperties();

    if (setupRuleMapLayers())
        setupRules();
}

AutoMapper::~AutoMapper() = default;

QString AutoMapper::rulesMapFileName() const
{
    return mRulesMap->fileName;
}

bool AutoMapper::ruleLayerNameUsed(const QString &ruleLayerName) const
{
    return mRuleMapSetup.mInputLayerNames.contains(ruleLayerName);
}

template<typename Type>
bool checkOption(const QString &propertyName,
                 const QVariant &propertyValue,
                 const QLatin1String optionName,
                 Type &member)
{
    if (propertyName.compare(optionName, Qt::CaseInsensitive) == 0) {
        if (propertyValue.canConvert<Type>()) {
            member = propertyValue.value<Type>();
            return true;
        }
    }
    return false;
}

template<typename Type>
bool checkRuleOption(const QString &propertyName,
                     const QVariant &propertyValue,
                     const QLatin1String optionName,
                     Type &member,
                     unsigned &setOptions,
                     RuleOptions::Enum optionFlag)
{
    if (checkOption(propertyName, propertyValue, optionName, member)) {
        setOptions |= optionFlag;
        return true;
    }
    return false;
}

static bool checkRuleOptions(const QString &name,
                             const QVariant &value,
                             RuleOptions &options,
                             unsigned &setOptions)
{
    if (checkRuleOption(name, value, QLatin1String("Probability"), options.skipChance, setOptions, RuleOptions::SkipChance)) {
        options.skipChance = 1.0 - options.skipChance;  // inverted so we don't have to keep inverting it later
        return true;
    }
    if (checkRuleOption(name, value, QLatin1String("ModX"), options.modX, setOptions, RuleOptions::ModX)) {
        options.modX = qMax<unsigned>(1, options.modX); // modulo 0 would crash
        return true;
    }
    if (checkRuleOption(name, value, QLatin1String("ModY"), options.modY, setOptions, RuleOptions::ModY)) {
        options.modY = qMax<unsigned>(1, options.modY); // modulo 0 would crash
        return true;
    }
    if (checkRuleOption(name, value, QLatin1String("OffsetX"), options.offsetX, setOptions, RuleOptions::OffsetX))
        return true;
    if (checkRuleOption(name, value, QLatin1String("OffsetY"), options.offsetY, setOptions, RuleOptions::OffsetY))
        return true;
    if (checkRuleOption(name, value, QLatin1String("NoOverlappingOutput"), options.noOverlappingOutput, setOptions, RuleOptions::NoOverlappingOutput))
        return true;
    if (checkRuleOption(name, value, QLatin1String("Disabled"), options.disabled, setOptions, RuleOptions::Disabled))
        return true;
    if (checkRuleOption(name, value, QLatin1String("IgnoreLock"), options.ignoreLock, setOptions, RuleOptions::IgnoreLock))
        return true;

    return false;
}

void AutoMapper::setupRuleMapProperties()
{
    unsigned setRuleOptions = 0;
    bool noOverlappingRules = false;

    QMapIterator<QString, QVariant> it(mRulesMap->properties());
    while (it.hasNext()) {
        it.next();

        const QString &name = it.key();
        const QVariant &value = it.value();

        if (checkOption(name, value, QLatin1String("DeleteTiles"), mOptions.deleteTiles))
            continue;
        if (checkOption(name, value, QLatin1String("MatchOutsideMap"), mOptions.matchOutsideMap))
            continue;
        if (checkOption(name, value, QLatin1String("OverflowBorder"), mOptions.overflowBorder))
            continue;
        if (checkOption(name, value, QLatin1String("WrapBorder"), mOptions.wrapBorder))
            continue;
        if (checkOption(name, value, QLatin1String("AutomappingRadius"), mOptions.autoMappingRadius))
            continue;
        if (checkOption(name, value, QLatin1String("NoOverlappingRules"), noOverlappingRules))
            continue;
        if (checkOption(name, value, QLatin1String("MatchInOrder"), mOptions.matchInOrder)) {
            mOptions.matchInOrderWasSet = true;
            continue;
        }

        if (checkRuleOptions(name, value, mRuleOptions, setRuleOptions))
            continue;

        addWarning(tr("Ignoring unknown property '%2' = '%3' (rule map '%1')")
                      .arg(rulesMapFileName(),
                           name,
                           value.toString()),
                   SelectCustomProperty { rulesMapFileName(), name, mRulesMap.get() });
    }

    // Each of the border options imply MatchOutsideMap
    if (mOptions.overflowBorder || mOptions.wrapBorder)
        mOptions.matchOutsideMap = true;

    // For backwards compatibility
    if (!(setRuleOptions & RuleOptions::NoOverlappingOutput))
        mRuleOptions.noOverlappingOutput = noOverlappingRules;
}

void AutoMapper::setupInputLayerProperties(InputLayer &inputLayer)
{
    QMapIterator<QString, QVariant> it(inputLayer.tileLayer->properties());
    while (it.hasNext()) {
        it.next();

        const QString &name = it.key();
        const QVariant &value = it.value();

        if (checkOption(name, value, QLatin1String("StrictEmpty"), inputLayer.strictEmpty))
            continue;
        if (checkOption(name, value, QLatin1String("AutoEmpty"), inputLayer.strictEmpty))
            continue;

        bool ignoreFlip;
        if (checkOption(name, value, QLatin1String("IgnoreHorizontalFlip"), ignoreFlip) && ignoreFlip) {
            inputLayer.flagsMask &= ~Cell::FlippedHorizontally;
            continue;
        }
        if (checkOption(name, value, QLatin1String("IgnoreVerticalFlip"), ignoreFlip) && ignoreFlip) {
            inputLayer.flagsMask &= ~Cell::FlippedVertically;
            continue;
        }
        if (checkOption(name, value, QLatin1String("IgnoreDiagonalFlip"), ignoreFlip) && ignoreFlip) {
            inputLayer.flagsMask &= ~Cell::FlippedAntiDiagonally;
            continue;
        }
        if (checkOption(name, value, QLatin1String("IgnoreHexRotate120"), ignoreFlip) && ignoreFlip) {
            inputLayer.flagsMask &= ~Cell::RotatedHexagonal120;
            continue;
        }

        addWarning(tr("Ignoring unknown property '%2' = '%3' on layer '%4' (rule map '%1')")
                   .arg(rulesMapFileName(),
                        name,
                        value.toString(),
                        inputLayer.tileLayer->name()),
                   SelectCustomProperty { rulesMapFileName(), name, inputLayer.tileLayer });
    }
}

void AutoMapper::setupOutputSetProperties(OutputSet &outputSet, RuleMapSetup &setup)
{
    for (const auto &outputLayer : outputSet.layers) {
        const Layer *layer = outputLayer.layer;

        Properties outputProperties;

        QMapIterator<QString, QVariant> properiesIterator(layer->properties());
        while (properiesIterator.hasNext()) {
            properiesIterator.next();

            const QString &name = properiesIterator.key();
            const QVariant &value = properiesIterator.value();

            if (name.compare(QLatin1String("probability"), Qt::CaseInsensitive) == 0) {
                bool ok;
                if (const qreal probability = value.toReal(&ok); ok) {
                    outputSet.probability = probability;
                    continue;
                }
            }

            // Unrecognized properties are copied to target map when a related rule matches
            outputProperties.insert(name, value);
        }

        if (!outputProperties.isEmpty())
            setup.mOutputLayerProperties[layer] = std::move(outputProperties);
    }
}

void AutoMapper::setupRuleOptionsArea(RuleOptionsArea &optionsArea, const MapObject *mapObject)
{
    QMapIterator<QString, QVariant> it(mapObject->properties());
    while (it.hasNext()) {
        it.next();

        const QString &name = it.key();
        const QVariant &value = it.value();

        if (checkRuleOptions(name, value, optionsArea.options, optionsArea.setOptions))
            continue;

        addWarning(tr("Ignoring unknown property '%2' = '%3' for rule options (rule map '%1')")
                   .arg(rulesMapFileName(),
                        name,
                        value.toString()),
                   SelectCustomProperty { rulesMapFileName(), name, mapObject });
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

        if (ruleMapLayerName.compare(QLatin1String("rule_options"), Qt::CaseInsensitive) == 0) {
            if (const ObjectGroup *objectGroup = layer->asObjectGroup()) {
                for (const MapObject *mapObject : *objectGroup) {
                    if (mapObject->shape() != MapObject::Rectangle || !mapObject->cell().isEmpty()) {
                        addWarning(tr("Only rectangle objects are supported on 'rule_options' layers."),
                                   JumpToObject { mapObject });
                        continue;
                    }

                    if (mapObject->rotation() != 0.0) {
                        addWarning(tr("Rotated rectangles are not supported on 'rule_options' layers."),
                                   JumpToObject { mapObject });
                        continue;
                    }

                    RuleOptionsArea &optionsArea = setup.mRuleOptionsAreas.emplace_back();
                    optionsArea.area = objectTileRect(*mRulesMapRenderer, *mapObject);
                    setupRuleOptionsArea(optionsArea, mapObject);
                }
            } else {
                error += tr("'rule_options' layers must be object layers.");
                error += QLatin1Char('\n');
            }
            continue;
        }

        const int layerNameStartPosition = ruleMapLayerName.indexOf(QLatin1Char('_'));

        // both 'rule' and 'output' layers will require and underscore and
        // rely on the correct position detected of the underscore
        if (layerNameStartPosition == -1) {
            error += tr("Did you forget an underscore in layer '%1'?").arg(ruleMapLayerName);
            error += QLatin1Char('\n');
            continue;
        }

        const QString layerName = ruleMapLayerName.mid(layerNameStartPosition + 1); // all characters after the underscore
        QString setName = ruleMapLayerName.left(layerNameStartPosition);            // all before the underscore

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

            InputLayer inputLayer { tileLayer };
            setupInputLayerProperties(inputLayer);

            auto &inputSet = find_or_emplace<InputSet>(setup.mInputSets, [&setName] (const InputSet &set) {
                return set.name == setName;
            }, setName);

            auto &conditions = find_or_emplace<InputConditions>(inputSet.layers, [&layerName] (const InputConditions &conditions) {
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
                setup.mOutputTileLayerNames.insert(layerName);
            else if (layer->isObjectGroup())
                setup.mOutputObjectGroupNames.insert(layerName);

            auto &outputSet = find_or_emplace<OutputSet>(setup.mOutputSets, [&setName] (const OutputSet &set) {
                return set.name == setName;
            }, setName);

            outputSet.layers.append({ layer, layerName });

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

    for (OutputSet &set : setup.mOutputSets)
        setupOutputSetProperties(set, setup);

    if (!error.isEmpty()) {
        error = rulesMapFileName() + QLatin1Char('\n') + error;
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

static void mergeRuleOptions(RuleOptions &self, const RuleOptions &other,
                             const unsigned setOptions)
{
    if (setOptions & RuleOptions::SkipChance)
        self.skipChance = other.skipChance;
    if (setOptions & RuleOptions::ModX)
        self.modX = other.modX;
    if (setOptions & RuleOptions::ModY)
        self.modY = other.modY;
    if (setOptions & RuleOptions::OffsetX)
        self.offsetX = other.offsetX;
    if (setOptions & RuleOptions::OffsetY)
        self.offsetY = other.offsetY;
    if (setOptions & RuleOptions::NoOverlappingOutput)
        self.noOverlappingOutput = other.noOverlappingOutput;
    if (setOptions & RuleOptions::Disabled)
        self.disabled = other.disabled;
    if (setOptions & RuleOptions::IgnoreLock)
        self.ignoreLock = other.ignoreLock;
}

void AutoMapper::setupRules()
{
    const auto &setup = mRuleMapSetup;

    Q_ASSERT(mRules.empty());

    QRegion regionInput;
    QRegion regionOutput;

    if (setup.mLayerRegions)
        regionInput = regionOutput = setup.mLayerRegions->region();
    if (setup.mLayerInputRegions)
        regionInput |= setup.mLayerInputRegions->region();
    if (setup.mLayerOutputRegions)
        regionOutput |= setup.mLayerOutputRegions->region();

    const bool legacyMode = (mRuleMapSetup.mLayerRegions ||
                             mRuleMapSetup.mLayerInputRegions);

    // When no input regions have been defined at all, derive them from the
    // "input" and "inputnot" layers.
    if (!setup.mLayerRegions && !setup.mLayerInputRegions) {
        for (const InputSet &inputSet : std::as_const(mRuleMapSetup.mInputSets)) {
            for (const InputConditions &conditions : inputSet.layers) {
                for (const InputLayer &inputLayer : conditions.listNo)
                    regionInput |= inputLayer.tileLayer->region();
                for (const InputLayer &inputLayer : conditions.listYes)
                    regionInput |= inputLayer.tileLayer->region();
            }
        }
    } else if (!mOptions.matchInOrderWasSet) {
        // For backwards compatibility, when the input regions have been
        // explicitly defined, we default MatchInOrder to true.
        mOptions.matchInOrder = true;
    }

    // When no output regions have been defined at all, derive them from the
    // "output" layers.
    if (!setup.mLayerRegions && !setup.mLayerOutputRegions) {
        for (const OutputSet &outputSet : std::as_const(mRuleMapSetup.mOutputSets)) {
            std::for_each(outputSet.layers.begin(),
                          outputSet.layers.end(),
                          [&] (const OutputLayer &outputLayer) {
                const Layer *layer = outputLayer.layer;
                if (layer->isTileLayer()) {
                    auto tileLayer = static_cast<const TileLayer*>(layer);
                    regionOutput |= tileLayer->region();
                } else if (layer->isObjectGroup()) {
                    auto objectGroup = static_cast<const ObjectGroup*>(layer);
                    regionOutput |= tileRegionOfObjectGroup(*mRulesMapRenderer,
                                                            objectGroup);
                }
            });
        }
    }

    QVector<QRegion> combinedRegions = coherentRegions(regionInput + regionOutput);

    // Combined regions are sorted, in order to have a deterministic order in
    // which the rules are applied.
    std::sort(combinedRegions.begin(), combinedRegions.end(), compareRuleRegion);

    // Then, they are split up in input and output region for each rule.
    mRules.reserve(combinedRegions.size());

    for (const QRegion &combinedRegion : combinedRegions) {
        Rule &rule = mRules.emplace_back();
        rule.inputRegion = combinedRegion & regionInput;
        rule.outputRegion = combinedRegion & regionOutput;
        rule.options = mRuleOptions;

        for (const auto &optionsArea : setup.mRuleOptionsAreas)
            if (combinedRegion.intersected(optionsArea.area) == combinedRegion)
                mergeRuleOptions(rule.options, optionsArea.options, optionsArea.setOptions);

        for (const OutputSet &outputSet : std::as_const(mRuleMapSetup.mOutputSets)) {
            RuleOutputSet index;
            if (compileOutputSet(index, outputSet, rule.outputRegion) || legacyMode) {
                if (outputSet.name.isEmpty() && !legacyMode)
                    rule.outputSet = std::move(index);
                else
                    rule.outputSets.add(index, outputSet.probability);
            }
        }
    }

#ifndef QT_NO_DEBUG
    for (const Rule &rule : mRules) {
        const QRegion checkCoherent = rule.inputRegion.united(rule.outputRegion);
        Q_ASSERT(coherentRegions(checkCoherent).size() == 1);
    }
#endif
}

void AutoMapper::prepareAutoMap(AutoMappingContext &context)
{
    setupWorkMapLayers(context);

    context.targetDocument->unifyTilesets(*mRulesMap, context.newTilesets);
}

/**
 * Makes sure all needed output layers are present in the working map.
 */
void AutoMapper::setupWorkMapLayers(AutoMappingContext &context) const
{
    // Set up pointers to output tile layers in the target map.
    // They are cloned when found in the target map, or created otherwise.
    for (const QString &name : std::as_const(mRuleMapSetup.mOutputTileLayerNames)) {
        auto &outputTileLayer = context.outputTileLayers[name];
        if (outputTileLayer)
            continue;

        if (const auto layer = context.targetMap->findLayer(name, Layer::TileLayerType)) {
            auto tileLayer = static_cast<TileLayer*>(layer);
            auto clone = std::unique_ptr<TileLayer>(tileLayer->clone());
            outputTileLayer = clone.get();
            context.originalToOutputLayerMapping[tileLayer] = std::move(clone);
        } else {
            auto newLayer = std::make_unique<TileLayer>(name, QPoint(), context.targetMap->size());
            outputTileLayer = newLayer.get();
            context.newLayers.push_back(std::move(newLayer));
        }
    }

    // Set up pointers to output object layers in the target map.
    // They are created when they are not present, but not cloned since objects
    // are not added directly.
    for (const QString &name : std::as_const(mRuleMapSetup.mOutputObjectGroupNames)) {
        auto &objectGroup = context.outputObjectGroups[name];
        if (objectGroup)
            continue;

        objectGroup = static_cast<ObjectGroup*>(context.targetMap->findLayer(name, Layer::ObjectGroupType));

        if (!objectGroup) {
            auto newLayer = std::make_unique<ObjectGroup>(name, 0, 0);
            objectGroup = newLayer.get();
            context.newLayers.push_back(std::move(newLayer));
        }
    }

    // Set up pointers to "set" layers (input layers in the target map). They
    // don't need to be created if not present.
    for (const QString &name : std::as_const(mRuleMapSetup.mInputLayerNames)) {
        // Check whether this input layer is also an output layer, in which
        // case we want to use its copy so we can see changes applied by
        // earlier rules.
        if (auto tileLayer = context.outputTileLayers.value(name))
            context.inputLayers.insert(name, tileLayer);
        else if (auto tileLayer = static_cast<TileLayer*>(context.targetMap->findLayer(name, Layer::TileLayerType)))
            context.inputLayers.insert(name, tileLayer);
    }
}

template<typename T>
static void appendUnique(QVector<T> &values, const T &value)
{
    if (!values.contains(value))
        values.append(value);
}

template<typename Callback>
static void forEachPointInRegion(const QRegion &region, Callback callback)
{
    for (const QRect &rect : region)
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                callback(x, y);
}

/**
 * Fills \a cells with the list of all cells which can be found within all
 * tile layers within the given region.
 *
 * Only collects tiles with MatchType::Tile.
 */
static void collectCellsInRegion(const QVector<InputLayer> &list,
                                 const QRegion &r,
                                 QVector<MatchCell> &cells)
{
    for (const InputLayer &inputLayer : list) {
        forEachPointInRegion(r, [&] (int x, int y) {
            const Cell &cell = inputLayer.tileLayer->cellAt(x, y);
            switch (matchType(cell.tile())) {
            case MatchType::Tile:
                appendUnique(cells, { cell, inputLayer.flagsMask });
                break;
            case MatchType::Empty:
                appendUnique(cells, MatchCell());
                break;
            default:
                break;
            }
        });
    }
}

static bool isEmptyRegion(const TileLayer &tileLayer,
                          const QRegion &region)
{
    for (const QRect &rect : region)
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                if (!tileLayer.cellAt(x, y).isEmpty())
                    return false;

    return true;
}

/**
 * Sets up a small data structure for this rule that is optimized for matching.
 */
bool AutoMapper::compileRule(QVector<RuleInputSet> &inputSets,
                             const Rule &rule,
                             const AutoMappingContext &context) const
{
    CompileContext compileContext;

    for (const InputSet &inputSet : std::as_const(mRuleMapSetup.mInputSets)) {
        RuleInputSet index;
        if (compileInputSet(index, inputSet, rule.inputRegion, compileContext, context))
            inputSets.append(std::move(index));
    }

    return !inputSets.isEmpty();
}

/**
 * After optimization, only one of \a anyOf or \a noneOf can contain any cells.
 *
 * Returns whether this combination can match at all. A match is not possible,
 * when \a anyOf is non-empty, but all cells in \a anyOf are also in \a noneOf.
 */
static bool optimizeAnyNoneOf(QVector<MatchCell> &anyOf, QVector<MatchCell> &noneOf)
{
    auto compareCell = [] (const MatchCell &a, const MatchCell &b) {
        if (a.tileset() != b.tileset())
            return a.tileset() < b.tileset();
        if (a.tileId() != b.tileId())
            return a.tileId() < b.tileId();
        if (a.flags() != b.flags())
            return a.flags() < b.flags();
        return a.flagsMask < b.flagsMask;
    };

    // First sort and erase duplicates
    if (!noneOf.isEmpty()) {
        std::stable_sort(noneOf.begin(), noneOf.end(), compareCell);
        noneOf.erase(std::unique(noneOf.begin(), noneOf.end()), noneOf.end());
    }

    // If there are any specific tiles desired, we don't need the noneOf
    if (!anyOf.isEmpty()) {
        std::stable_sort(anyOf.begin(), anyOf.end(), compareCell);
        anyOf.erase(std::unique(anyOf.begin(), anyOf.end()), anyOf.end());

        // Remove all "noneOf" tiles from "anyOf" and clear "noneOf"
        for (auto i = anyOf.begin(), j = noneOf.begin(); i != anyOf.end() && j != noneOf.end();) {
            if (compareCell(*i, *j)) {
                ++i;
            } else if (compareCell(*j, *i)) {
                ++j;
            } else {
                i = anyOf.erase(i);
                ++j;
            }
        }
        noneOf.clear();

        // If now no tiles are allowed anymore, this rule can never match
        if (anyOf.isEmpty())
            return false;
    }

    return true;
}

/**
 * Compiles one of a rule's input sets.
 *
 * Returns false when it detects a missing input layer would prevent the input
 * set from ever matching, or when a rule contradicts itself.
 */
bool AutoMapper::compileInputSet(RuleInputSet &index,
                                 const InputSet &inputSet,
                                 const QRegion &inputRegion,
                                 CompileContext &compileContext,
                                 const AutoMappingContext &context) const
{
    const QPoint topLeft = inputRegion.boundingRect().topLeft();

    QVector<MatchCell> &anyOf = compileContext.anyOf;
    QVector<MatchCell> &noneOf = compileContext.noneOf;
    QVector<MatchCell> &inputCells = compileContext.inputCells;

    for (const InputConditions &conditions : inputSet.layers) {
        inputCells.clear();
        bool canMatch = true;

        RuleInputLayer layer;
        layer.targetLayer = context.inputLayers.value(conditions.layerName, &dummy);

        forEachPointInRegion(inputRegion, [&] (int x, int y) {
            anyOf.clear();
            noneOf.clear();

            bool negate = false;

            for (const InputLayer &inputLayer : conditions.listYes) {
                const Cell &cell = inputLayer.tileLayer->cellAt(x, y);

                switch (matchType(cell.tile())) {
                case MatchType::Unknown:
                    if (inputLayer.strictEmpty)
                        anyOf.append({ cell, inputLayer.flagsMask });
                    break;
                case MatchType::Tile:
                    anyOf.append({ cell, inputLayer.flagsMask });
                    break;
                case MatchType::Empty:
                    anyOf.append(MatchCell());
                    break;
                case MatchType::NonEmpty:
                    noneOf.append(MatchCell());
                    break;
                case MatchType::Other:
                    // The "any other tile" case is implemented as "none of the
                    // used tiles".
                    if (inputCells.isEmpty())
                        collectCellsInRegion(conditions.listYes, inputRegion, inputCells);
                    noneOf.append(inputCells);
                    break;
                case MatchType::Negate:
                    negate = true;
                    break;
                case MatchType::Ignore:
                    break;
                }
            }

            for (const InputLayer &inputLayer : conditions.listNo) {
                const Cell &cell = inputLayer.tileLayer->cellAt(x, y);

                switch (matchType(cell.tile())) {
                case MatchType::Unknown:
                    if (inputLayer.strictEmpty)
                        noneOf.append({ cell, inputLayer.flagsMask });
                    break;
                case MatchType::Tile:
                    noneOf.append({ cell, inputLayer.flagsMask });
                    break;
                case MatchType::Empty:
                    noneOf.append(MatchCell());
                    break;
                case MatchType::NonEmpty:
                    anyOf.append(MatchCell());
                    break;
                case MatchType::Other:
                    // This is the "not any other tile" case, which is
                    // implemented as "any of the used tiles"
                    if (inputCells.isEmpty())
                        collectCellsInRegion(conditions.listYes, inputRegion, inputCells);
                    anyOf.append(inputCells);
                    break;
                case MatchType::Negate:
                    negate = true;
                    break;
                case MatchType::Ignore:
                    break;
                }
            }

            // For backwards compatibility, when the input regions have been
            // explicitly defined and no "any" and no "none" tiles are defined
            // at this location, the rule will not accept any of the "any"
            // tiles used elsewhere in this rule, nor the empty tile.
            if (mRuleMapSetup.mLayerRegions || mRuleMapSetup.mLayerInputRegions) {
                if (anyOf.isEmpty() && conditions.listNo.isEmpty()) {
                    if (inputCells.isEmpty())
                        collectCellsInRegion(conditions.listYes, inputRegion, inputCells);
                    noneOf.append(inputCells);
                    noneOf.append(MatchCell());
                }
            }

            if (negate)
                std::swap(anyOf, noneOf);

            if (!optimizeAnyNoneOf(anyOf, noneOf)) {
                canMatch = false;
                return;
            }

            // When the input layer is missing, it is considered empty. In this
            // case, we can drop this input set when empty tiles are not
            // allowed here.
            if (layer.targetLayer == &dummy) {
                const bool emptyAllowed = (anyOf.isEmpty() ||
                                           std::any_of(anyOf.cbegin(),
                                                       anyOf.cend(),
                                                       [] (const MatchCell &cell) { return cell.isEmpty(); }))
                        && std::none_of(noneOf.cbegin(),
                                        noneOf.cend(),
                                        [] (const MatchCell &cell) { return cell.isEmpty(); });

                if (!emptyAllowed)
                    canMatch = false;
            }

            if (!anyOf.empty() || !noneOf.empty()) {
                index.cells.append(anyOf);
                index.cells.append(noneOf);

                RuleInputLayerPos pos;
                pos.x = x - topLeft.x();
                pos.y = y - topLeft.y();
                pos.anyCount = anyOf.size();
                pos.noneCount = noneOf.size();

                index.positions.append(pos);
                ++layer.posCount;
            }
        });

        if (!canMatch)
            return false;

        if (layer.posCount > 0)
            index.layers.append(layer);
    }

    return true;
}

/**
 * Processes the given \a outputSet, adding the output layers to the given
 * \a index. Returns whether the output set is non-empty.
 */
bool AutoMapper::compileOutputSet(RuleOutputSet &index,
                                  const OutputSet &outputSet,
                                  const QRegion &outputRegion) const
{
    for (const OutputLayer &outputLayer : outputSet.layers) {
        const Layer *from = outputLayer.layer;

        switch (from->layerType()) {
        case Layer::TileLayerType: {
            auto fromTileLayer = static_cast<const TileLayer*>(from);

            if (!isEmptyRegion(*fromTileLayer, outputRegion))
                index.tileOutputs.append({ fromTileLayer, outputLayer.name });
            break;
        }
        case Layer::ObjectGroupType: {
            auto fromObjectGroup = static_cast<const ObjectGroup*>(from);

            auto objects = objectsInRegion(*mRulesMapRenderer, fromObjectGroup, outputRegion);
            if (!objects.isEmpty()) {
                QVector<const MapObject*> constObjects;
                for (auto object : objects)
                    constObjects.append(object);
                index.objectOutputs.append({ fromObjectGroup, constObjects, outputLayer.name });
            }
            break;
        }
        case Layer::ImageLayerType:
        case Layer::GroupLayerType:
            Q_UNREACHABLE();
            break;
        }
    }

    return !(index.tileOutputs.isEmpty() && index.objectOutputs.isEmpty());
}

void AutoMapper::autoMap(const QRegion &where,
                         QRegion *appliedRegion,
                         AutoMappingContext &context) const
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
        for (const QString &name : std::as_const(mRuleMapSetup.mInputLayerNames)) {
            if (const TileLayer *inputLayer = context.inputLayers.value(name))
                inputLayersRegion |= inputLayer->region();
        }

        const QRegion regionToErase = inputLayersRegion.intersected(applyRegion);

        for (const QString &name : mRuleMapSetup.mOutputTileLayerNames)
            context.outputTileLayers.value(name)->erase(regionToErase);

        for (const QString &name : mRuleMapSetup.mOutputObjectGroupNames) {
            const auto objects = objectsInRegion(*context.targetDocument->renderer(),
                                                 context.outputObjectGroups.value(name),
                                                 regionToErase);
            for (MapObject *mapObject : objects)
                context.mapObjectsToRemove.insert(mapObject);
        }
    }

    // Those two options are guaranteed to be false if the map is infinite,
    // so no "invalid" width/height accessing here.
    GetCell get = &getCell;
    if (!context.targetMap->infinite()) {
        if (mOptions.wrapBorder)
            get = &getWrappedCell;
        else if (mOptions.overflowBorder)
            get = &getBoundCell;
    }

    ApplyContext applyContext { appliedRegion };

    if (mOptions.matchInOrder) {
        for (const Rule &rule : mRules) {
            if (rule.options.disabled)
                continue;

            matchRule(rule, applyRegion, get, [&] (QPoint pos) {
                applyRule(rule, pos, applyContext, context);
            }, context);
            applyContext.appliedRegions.clear();
        }
    } else {
        auto collectMatches = [&] (const Rule &rule) {
            QVector<QPoint> positions;
            if (!rule.options.disabled)
                matchRule(rule, applyRegion, get, [&] (QPoint pos) { positions.append(pos); }, context);
            return positions;
        };
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto result = QtConcurrent::blockingMapped(mRules, collectMatches);
#else
        struct MatchRule
        {
            using result_type = QVector<QPoint>;

            std::function<result_type(const Rule&)> collectMatches;

            result_type operator()(const Rule &rule)
            {
                return collectMatches(rule);
            }
        };

        const auto result = QtConcurrent::blockingMapped<QVector<QVector<QPoint>>>(mRules,
                                                                                   MatchRule { collectMatches });
#endif

        for (size_t i = 0; i < mRules.size(); ++i) {
            const Rule &rule = mRules[i];
            for (const QPoint pos : result[i])
                applyRule(rule, pos, applyContext, context);
            applyContext.appliedRegions.clear();
        }
    }
}

static bool cellMatches(const MatchCell &matchCell, const Cell &cell)
{
    const auto flagsMask = matchCell.flagsMask;
    return matchCell.tileset() == cell.tileset()
            && matchCell.tileId() == cell.tileId()
            && (matchCell.flags() & flagsMask) == (cell.flags() & flagsMask);
}

/**
 * Checks whether the given \a inputSet matches at the given \a offset.
 */
static bool matchInputIndex(const RuleInputSet &inputSet, QPoint offset, AutoMapper::GetCell getCell)
{
    qsizetype nextPos = 0;
    qsizetype nextCell = 0;

    for (const RuleInputLayer &layer : inputSet.layers) {
        for (auto p = std::exchange(nextPos, nextPos + layer.posCount); p < nextPos; ++p) {
            const RuleInputLayerPos &pos = inputSet.positions[p];
            const Cell &cell = getCell(pos.x + offset.x(), pos.y + offset.y(), *layer.targetLayer);

            // Match may succeed if any of the "any" tiles are seen, or when
            // there are no "any" tiles for this location.
            bool anyMatch = !pos.anyCount;

            for (auto c = std::exchange(nextCell, nextCell + pos.anyCount); c < nextCell; ++c) {
                const MatchCell &desired = inputSet.cells[c];
                if (desired.isEmpty() ? cell.isEmpty() : cellMatches(desired, cell)) {
                    anyMatch = true;
                    break;
                }
            }

            if (!anyMatch)
                return false;

            // Match fails as soon as any of the "none" tiles is seen
            for (auto c = std::exchange(nextCell, nextCell + pos.noneCount); c < nextCell; ++c) {
                const MatchCell &undesired = inputSet.cells[c];
                if (undesired.isEmpty() ? cell.isEmpty() : cellMatches(undesired, cell))
                    return false;
            }
        }
    }

    return true;
}

static bool matchRuleAtOffset(const QVector<RuleInputSet> &inputSets, QPoint offset, AutoMapper::GetCell getCell)
{
    return std::any_of(inputSets.begin(),
                       inputSets.end(),
                       [=] (const RuleInputSet &index) { return matchInputIndex(index, offset, getCell); });
}

void AutoMapper::matchRule(const Rule &rule,
                           const QRegion &matchRegion,
                           GetCell getCell,
                           const std::function<void(QPoint pos)> &matched,
                           const AutoMappingContext &context) const
{
    if (!rule.outputSet && rule.outputSets.isEmpty())
        return;

    QVector<RuleInputSet> inputSets;
    if (!compileRule(inputSets, rule, context))
        return;

    const QRect inputBounds = rule.inputRegion.boundingRect();

    // This is really the rule size - 1, since when applying the rule we will
    // keep at least one tile overlap with the apply region.
    const int ruleWidth = inputBounds.right() - inputBounds.left();
    const int ruleHeight = inputBounds.bottom() - inputBounds.top();

    QRegion ruleMatchRegion;

    for (const QRect &rect : matchRegion) {
        // Expand each rect, making sure that there is at least one tile
        // overlap with the rule at all sides.
        ruleMatchRegion |= rect.adjusted(-ruleWidth, -ruleHeight, 0, 0);
    }

    // When we're not matching a rule outside the map, we reduce the region in
    // in which it is applied accordingly.
    if (!mOptions.matchOutsideMap && !context.targetMap->infinite()) {
        ruleMatchRegion &= QRect(0, 0,
                                 context.targetMap->width() - ruleWidth,
                                 context.targetMap->height() - ruleHeight);
    }

    for (const QRect &rect : ruleMatchRegion) {
        const int startX = rect.left() + (rect.left() + rule.options.offsetX) % rule.options.modX;
        const int startY = rect.top() + (rect.top() + rule.options.offsetY) % rule.options.modY;

        for (int y = startY; y <= rect.bottom(); y += rule.options.modY) {
            for (int x = startX; x <= rect.right(); x += rule.options.modX) {
                if (rule.options.skipChance != 0.0 && randomDouble() < rule.options.skipChance)
                    continue;

                if (matchRuleAtOffset(inputSets, QPoint(x, y), getCell))
                    matched(QPoint(x, y));
            }
        }
    }
}

void AutoMapper::applyRule(const Rule &rule, QPoint pos,
                           ApplyContext &applyContext,
                           AutoMappingContext &context) const
{
    // Translate the position to adjust to the location of the rule.
    pos -= rule.inputRegion.boundingRect().topLeft();

    // If named output sets are given, choose one of them by chance
    const RuleOutputSet *randomOutputSet = nullptr;
    if (!rule.outputSets.isEmpty())
        randomOutputSet = &rule.outputSets.pick();

    if (rule.options.noOverlappingOutput) {
        QHash<const Layer*, QRegion> ruleRegionInLayer;

        if (rule.outputSet)
            collectLayerOutputRegions(rule, *rule.outputSet, context, ruleRegionInLayer);

        if (randomOutputSet)
            collectLayerOutputRegions(rule, *randomOutputSet, context, ruleRegionInLayer);

        // Translate the regions to the position of the rule and check for overlap.
        for (auto it = ruleRegionInLayer.keyValueBegin(), it_end = ruleRegionInLayer.keyValueEnd();
             it != it_end; ++it) {

            auto base = it.base();
            const Layer *layer = base.key();
            QRegion &region = base.value();

            region.translate(pos.x(), pos.y());

            if (applyContext.appliedRegions[layer].intersects(region))
                return; // Don't apply the rule
        }

        // Remember the newly applied region
        for (auto it = ruleRegionInLayer.keyValueBegin(), it_end = ruleRegionInLayer.keyValueEnd();
             it != it_end; ++it) {

            auto base = it.base();
            const Layer *layer = base.key();
            const QRegion &region = base.value();

            applyContext.appliedRegions[layer] |= region;
        }
    }

    if (rule.outputSet)
        copyMapRegion(rule, pos, *rule.outputSet, context);

    if (randomOutputSet)
        copyMapRegion(rule, pos, *randomOutputSet, context);

    if (applyContext.appliedRegion)
        *applyContext.appliedRegion |= rule.outputRegion.translated(pos.x(), pos.y());
}

/**
 * Collects the per-layer output region of the given \a rule, when using the
 * given \a outputSet.
 *
 * The \a ruleRegionInLayer parameter tells us for each target output layer,
 * which region will be touched by applying this output.
 */
void AutoMapper::collectLayerOutputRegions(const Rule &rule,
                                           const RuleOutputSet &outputSet,
                                           AutoMappingContext &context,
                                           QHash<const Layer*, QRegion> &ruleRegionInLayer) const
{
    // TODO: Very slow to re-calculate the entire region for each rule output
    // layer here, each time a rule has a match. These regions are also
    // calculated in AutoMapper::setupRules, when no region layers are defined.

    for (const auto &tileOutput : outputSet.tileOutputs) {
        const Layer *targetLayer = context.outputTileLayers.value(tileOutput.name);
        QRegion &outputLayerRegion = ruleRegionInLayer[targetLayer];
        outputLayerRegion |= tileOutput.tileLayer->region() & rule.outputRegion;
    }

    for (const auto &objectOutput : outputSet.objectOutputs) {
        const Layer *targetLayer = context.outputTileLayers.value(objectOutput.name);
        QRegion &outputLayerRegion = ruleRegionInLayer[targetLayer];
        for (const MapObject *mapObject : objectOutput.objects)
            outputLayerRegion |= objectTileRect(*mRulesMapRenderer, *mapObject);
    }
}

void AutoMapper::copyMapRegion(const Rule &rule, QPoint offset,
                               const RuleOutputSet &outputSet,
                               AutoMappingContext &context) const
{
    for (const auto &tileOutput : outputSet.tileOutputs) {
        TileLayer *toTileLayer = context.outputTileLayers.value(tileOutput.name);

        if (!rule.options.ignoreLock && !toTileLayer->isUnlocked())
            continue;

        if (!context.touchedTileLayers.isEmpty())
            appendUnique<const TileLayer*>(context.touchedTileLayers, toTileLayer);

        for (const QRect &rect : rule.outputRegion) {
            copyTileRegion(tileOutput.tileLayer, rect, toTileLayer,
                           rect.x() + offset.x(), rect.y() + offset.y(),
                           context);
        }

        applyLayerProperties(tileOutput.tileLayer, toTileLayer, context);
    }

    if (!outputSet.objectOutputs.isEmpty()) {
        QVector<AddMapObjects::Entry> newMapObjects;
        newMapObjects.reserve(outputSet.objectOutputs.size());

        const MapRenderer *renderer = context.targetDocument->renderer();
        const QRect outputRect = rule.outputRegion.boundingRect();
        const QRectF pixelRect = renderer->tileToPixelCoords(outputRect);
        const QPointF pixelOffset = renderer->tileToPixelCoords(outputRect.topLeft() + offset) - pixelRect.topLeft();

        for (const auto &objectOutput : outputSet.objectOutputs) {
            auto toObjectGroup = context.outputObjectGroups.value(objectOutput.name);

            if (!rule.options.ignoreLock && !toObjectGroup->isUnlocked())
                continue;

            for (auto mapObject : objectOutput.objects) {
                MapObject *clone = mapObject->clone();
                clone->setX(clone->x() + pixelOffset.x());
                clone->setY(clone->y() + pixelOffset.y());
                newMapObjects.append(AddMapObjects::Entry { clone, toObjectGroup });
            }

            context.newMapObjects.append(newMapObjects);

            applyLayerProperties(objectOutput.objectGroup, toObjectGroup, context);
        }
    }
}

void AutoMapper::applyLayerProperties(const Layer *from, Layer *to, AutoMappingContext &context) const
{
    auto propertiesIt = mRuleMapSetup.mOutputLayerProperties.constFind(from);
    if (propertiesIt != mRuleMapSetup.mOutputLayerProperties.constEnd()) {
        Properties mergedProperties = context.changedProperties.value(to, to->properties());
        mergeProperties(mergedProperties, *propertiesIt);

        if (mergedProperties != to->properties()) {
            const bool isNewLayer = contains_where(context.newLayers,
                                                   [to] (const std::unique_ptr<Layer> &l) {
                return l.get() == to;
            });

            if (isNewLayer)
                to->setProperties(mergedProperties);
            else
                context.changedProperties.insert(to, mergedProperties);
        }
    }
}

void AutoMapper::copyTileRegion(const TileLayer *srcLayer, QRect rect,
                                TileLayer *dstLayer, int dstX, int dstY,
                                const AutoMappingContext &context) const
{
    int startX = dstX;
    int startY = dstY;

    int endX = dstX + rect.width();
    int endY = dstY + rect.height();

    const int dwidth = dstLayer->width();
    const int dheight = dstLayer->height();

    const bool fixedSize = !context.targetMap->infinite();
    const bool wrapBorder = mOptions.wrapBorder && fixedSize;
    if (!wrapBorder && fixedSize) {
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

            // this is without graphics update, it's done afterwards for all
            int xd = x;
            int yd = y;

            if (wrapBorder) {
                xd = wrap(xd, dwidth);
                yd = wrap(yd, dheight);
            }

            switch (matchType(cell.tile())) {
            case MatchType::Tile:
                dstLayer->setCell(xd, yd, cell);
                break;
            case MatchType::Empty:
                dstLayer->setCell(xd, yd, Cell());
                break;
            default:
                break;
            }
        }
    }
}

void AutoMapper::addWarning(const QString &message, std::function<void ()> callback)
{
    WARNING(message, std::move(callback));
    mWarning += message;
    mWarning += QLatin1Char('\n');
}

} // namespace Tiled

#include "moc_automapper.cpp"
