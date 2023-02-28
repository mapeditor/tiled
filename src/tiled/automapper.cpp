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
#include "object.h"
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
    else if (matchType == QLatin1String("NonEmpty"))
        return MatchType::NonEmpty;
    else if (matchType == QLatin1String("Other"))
        return MatchType::Other;
    else if (matchType == QLatin1String("Negate"))
        return MatchType::Negate;
    else if (matchType == QLatin1String("Ignore"))
        return MatchType::Ignore;

    return MatchType::Tile;
}

/**
 * The compile context enables re-using temporarily allocated memory while
 * compiling the rules.
 */
struct CompileContext
{
    QVector<Cell> anyOf;
    QVector<Cell> noneOf;
    QVector<Cell> inputCells;
};

struct ApplyContext
{
    ApplyContext(QRegion *appliedRegion)
        : appliedRegion(appliedRegion)
    {}

    // These regions store which parts or the map have already been altered by
    // exactly this rule. We store all the altered parts to make sure there are
    // no overlaps of the same rule applied to (neighbouring) places.
    QHash<const Layer*, QRegion> appliedRegions;

    QRandomGenerator *randomGenerator = QRandomGenerator::global();

    QRegion *appliedRegion;
};


AutoMappingContext::AutoMappingContext(MapDocument *mapDocument)
    : targetDocument(mapDocument)
    , targetMap(targetDocument->map())
{
}


AutoMapper::AutoMapper(std::unique_ptr<Map> rulesMap, const QRegularExpression &mapNameFilter)
    : mRulesMap(std::move(rulesMap))
    , mMapNameFilter(mapNameFilter)
{
    Q_ASSERT(mRulesMap);

    setupRuleMapProperties();

    if (setupRuleMapLayers())
        setupRules();
}

AutoMapper::~AutoMapper()
{
}

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
    inputLayer.strictEmpty = false;

    QMapIterator<QString, QVariant> it(inputLayer.tileLayer->properties());
    while (it.hasNext()) {
        it.next();

        const QString &name = it.key();
        const QVariant &value = it.value();

        if (name.compare(QLatin1String("strictempty"), Qt::CaseInsensitive) == 0 ||
                name.compare(QLatin1String("autoempty"), Qt::CaseInsensitive) == 0) {
            if (value.canConvert(QMetaType::Bool)) {
                inputLayer.strictEmpty = value.toBool();
                continue;
            }
        }

        addWarning(tr("Ignoring unknown property '%2' = '%3' on layer '%4' (rule map '%1')")
                   .arg(rulesMapFileName(),
                        name,
                        value.toString(),
                        inputLayer.tileLayer->name()),
                   SelectCustomProperty { rulesMapFileName(), name, inputLayer.tileLayer });
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
    const auto renderer = MapRenderer::create(mRulesMap.get());

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
                    optionsArea.area = QRectF(renderer->pixelToTileCoords(mapObject->bounds().topLeft()),
                                              renderer->pixelToTileCoords(mapObject->bounds().bottomRight())).toAlignedRect();
                    setupRuleOptionsArea(optionsArea, mapObject);
                }
            } else {
                error += tr("'rule_options' layers must be object layers.");
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
                setup.mOutputTileLayerNames.insert(layerName);
            else if (layer->isObjectGroup())
                setup.mOutputObjectGroupNames.insert(layerName);

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
            std::for_each(outputSet.layers.keyBegin(),
                          outputSet.layers.keyEnd(),
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
    mRules.reserve(combinedRegions.size());

    for (const QRegion &combinedRegion : combinedRegions) {
        Rule &rule = mRules.emplace_back();
        rule.inputRegion = combinedRegion & regionInput;
        rule.outputRegion = combinedRegion & regionOutput;
        rule.options = mRuleOptions;

        for (const auto &optionsArea : setup.mRuleOptionsAreas)
            if (combinedRegion.intersected(optionsArea.area) == combinedRegion)
                mergeRuleOptions(rule.options, optionsArea.options, optionsArea.setOptions);
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
                                 QVector<Cell> &cells)
{
    for (const InputLayer &inputLayer : list) {
        forEachPointInRegion(r, [&] (int x, int y) {
            const Cell &cell = inputLayer.tileLayer->cellAt(x, y);
            if (matchType(cell.tile()) == MatchType::Tile)
                appendUnique(cells, cell);
        });
    }
}

/**
 * Sets up a small data structure for this rule that is optimized for matching.
 */
void AutoMapper::compileRule(QVector<RuleInputSet> &inputSets,
                             const Rule &rule,
                             const AutoMappingContext &context) const
{
    CompileContext compileContext;

    for (const InputSet &inputSet : std::as_const(mRuleMapSetup.mInputSets)) {
        RuleInputSet index;
        if (compileInputSet(index, inputSet, rule.inputRegion, compileContext, context))
            inputSets.append(std::move(index));
    }
}

/**
 * After optimization, only one of \a anyOf or \a noneOf can contain any cells.
 *
 * Returns whether this combination can match at all. A match is not possible,
 * when \a anyOf is non-empty, but all cells in \a anyOf are also in \a noneOf.
 */
static bool optimizeAnyNoneOf(QVector<Cell> &anyOf, QVector<Cell> &noneOf)
{
    auto compareCell = [] (const Cell &a, const Cell &b) {
        if (a.tileset() != b.tileset())
            return a.tileset() < b.tileset();
        if (a.tileId() != b.tileId())
            return a.tileId() < b.tileId();
        return a.flags() < b.flags();
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

    QVector<Cell> &anyOf = compileContext.anyOf;
    QVector<Cell> &noneOf = compileContext.noneOf;
    QVector<Cell> &inputCells = compileContext.inputCells;

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
                        anyOf.append(cell);
                    break;
                case MatchType::Tile:
                    anyOf.append(cell);
                    break;
                case MatchType::Empty:
                    anyOf.append(Cell());
                    break;
                case MatchType::NonEmpty:
                    noneOf.append(Cell());
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
                        noneOf.append(cell);
                    break;
                case MatchType::Tile:
                    noneOf.append(cell);
                    break;
                case MatchType::Empty:
                    noneOf.append(Cell());
                    break;
                case MatchType::NonEmpty:
                    anyOf.append(Cell());
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
                    noneOf.append(Cell());
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
                                                       [] (const Cell &cell) { return cell.isEmpty(); }))
                        && std::none_of(noneOf.cbegin(),
                                        noneOf.cend(),
                                        [] (const Cell &cell) { return cell.isEmpty(); });

                if (!emptyAllowed)
                    canMatch = false;
            }

            if (anyOf.size() > 0 || noneOf.size() > 0) {
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
        for (const OutputSet &ruleOutput : std::as_const(mRuleMapSetup.mOutputSets)) {
            QHashIterator<const Layer*, QString> it(ruleOutput.layers);
            while (it.hasNext()) {
                it.next();

                const QString &name = it.value();

                switch (it.key()->layerType()) {
                case Layer::TileLayerType:
                    context.outputTileLayers.value(name)->erase(regionToErase);
                    break;
                case Layer::ObjectGroupType: {
                    const auto objects = objectsToErase(context.targetDocument,
                                                        context.outputObjectGroups.value(name),
                                                        regionToErase);
                    for (MapObject *mapObject : objects)
                        context.mapObjectsToRemove.insert(mapObject);
                    break;
                }
                case Layer::ImageLayerType:
                case Layer::GroupLayerType:
                    Q_UNREACHABLE();
                    break;
                }
            }
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

    ApplyContext applyContext(appliedRegion);

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
                const Cell &desired = inputSet.cells[c];
                if (desired.isEmpty() ? cell.isEmpty() : desired == cell) {
                    anyMatch = true;
                    break;
                }
            }

            if (!anyMatch)
                return false;

            // Match fails as soon as any of the "none" tiles is seen
            for (auto c = std::exchange(nextCell, nextCell + pos.noneCount); c < nextCell; ++c) {
                const Cell &undesired = inputSet.cells[c];
                if (undesired.isEmpty() ? cell.isEmpty() : undesired == cell)
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
    QVector<RuleInputSet> inputSets;
    compileRule(inputSets, rule, context);

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
    Q_ASSERT(!mRuleMapSetup.mOutputSets.empty());

    // Translate the position to adjust to the location of the rule.
    pos -= rule.inputRegion.boundingRect().topLeft();

    // choose by chance which group of rule_layers should be used:
    const int r = applyContext.randomGenerator->generate() % mRuleMapSetup.mOutputSets.size();
    const OutputSet &ruleOutput = mRuleMapSetup.mOutputSets.at(r);

    if (rule.options.noOverlappingOutput) {
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

            outputLayerRegion &= rule.outputRegion;
            outputLayerRegion.translate(pos.x(), pos.y());

            ruleRegionInLayer[layer] = outputLayerRegion;

            return applyContext.appliedRegions[layer].intersects(outputLayerRegion);
        });

        if (overlap)
            return;

        // Remember the newly applied region
        std::for_each(ruleOutput.layers.keyBegin(),
                      ruleOutput.layers.keyEnd(),
                      [&] (const Layer *layer) {
            applyContext.appliedRegions[layer] |= ruleRegionInLayer[layer];
        });
    }

    copyMapRegion(rule, pos, ruleOutput, context);

    if (applyContext.appliedRegion)
        *applyContext.appliedRegion |= rule.outputRegion.translated(pos.x(), pos.y());
}

void AutoMapper::copyMapRegion(const Rule &rule, QPoint offset,
                               const OutputSet &ruleOutput,
                               AutoMappingContext &context) const
{
    const QRegion &outputRegion = rule.outputRegion;

    for (auto it = ruleOutput.layers.begin(), end = ruleOutput.layers.end(); it != end; ++it) {
        const Layer *from = it.key();
        const QString &targetName = it.value();

        Layer *to = nullptr;

        switch (from->layerType()) {
        case Layer::TileLayerType: {
            auto fromTileLayer = static_cast<const TileLayer*>(from);
            auto toTileLayer = context.outputTileLayers.value(targetName);

            if (!rule.options.ignoreLock && !toTileLayer->isUnlocked())
                continue;

            if (!context.touchedTileLayers.isEmpty())
                appendUnique<const TileLayer*>(context.touchedTileLayers, toTileLayer);

            to = toTileLayer;
            for (const QRect &rect : outputRegion) {
                copyTileRegion(fromTileLayer, rect, toTileLayer,
                               rect.x() + offset.x(), rect.y() + offset.y(),
                               context);
            }
            break;
        }
        case Layer::ObjectGroupType: {
            auto fromObjectGroup = static_cast<const ObjectGroup*>(from);
            auto toObjectGroup = context.outputObjectGroups.value(targetName);

            if (!rule.options.ignoreLock && !toObjectGroup->isUnlocked())
                continue;

            to = toObjectGroup;
            for (const QRect &rect : outputRegion) {
                copyObjectRegion(fromObjectGroup, rect, toObjectGroup,
                                 rect.x() + offset.x(), rect.y() + offset.y(),
                                 context);
            }
            break;
        }
        case Layer::ImageLayerType:
        case Layer::GroupLayerType:
            Q_UNREACHABLE();
            break;
        }
        Q_ASSERT(to);

        // Copy any custom properties set on the output layer
        if (!from->properties().isEmpty()) {
            Properties mergedProperties = context.changedProperties.value(to, to->properties());
            mergeProperties(mergedProperties, from->properties());

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
            case Tiled::MatchType::Tile:
                dstLayer->setCell(xd, yd, cell);
                break;
            case Tiled::MatchType::Empty:
                dstLayer->setCell(xd, yd, Cell());
                break;
            default:
                break;
            }
        }
    }
}

void AutoMapper::copyObjectRegion(const ObjectGroup *srcLayer, const QRectF &rect,
                                  ObjectGroup *dstLayer, int dstX, int dstY,
                                  AutoMappingContext &context) const
{
    const QRectF pixelRect = context.targetDocument->renderer()->tileToPixelCoords(rect);
    const QList<MapObject*> objects = objectsInRegion(srcLayer, pixelRect.toAlignedRect());
    if (objects.isEmpty())
        return;

    QPointF pixelOffset = context.targetDocument->renderer()->tileToPixelCoords(dstX, dstY);
    pixelOffset -= pixelRect.topLeft();

    QVector<AddMapObjects::Entry> newMapObjects;
    newMapObjects.reserve(objects.size());

    for (MapObject *obj : objects) {
        MapObject *clone = obj->clone();
        clone->setX(clone->x() + pixelOffset.x());
        clone->setY(clone->y() + pixelOffset.y());
        newMapObjects.append(AddMapObjects::Entry { clone, dstLayer });
    }

    context.newMapObjects.append(newMapObjects);
}

void AutoMapper::addWarning(const QString &message, std::function<void ()> callback)
{
    WARNING(message, std::move(callback));
    mWarning += message;
    mWarning += QLatin1Char('\n');
}

} // namespace Tiled

#include "moc_automapper.cpp"
