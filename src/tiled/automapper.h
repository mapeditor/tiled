/*
 * automapper.h
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

#pragma once

#include "addremovemapobject.h"
#include "tilededitor_global.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QList>
#include <QMap>
#include <QRegion>
#include <QSet>
#include <QString>
#include <QVector>

#include <memory>
#include <type_traits>
#include <vector>

namespace Tiled {

class Layer;
class Map;
class MapObject;
class ObjectGroup;
class TileLayer;

class MapDocument;

struct InputLayer
{
    const TileLayer *tileLayer;
    bool strictEmpty;
};

struct InputConditions
{
    InputConditions(const QString &layerName) : layerName(layerName) {}

    QString layerName;
    QVector<InputLayer> listYes;        // "input"
    QVector<InputLayer> listNo;         // "inputnot"
};

struct InputSet
{
    InputSet(const QString &name) : name(name) {}

    QString name;
    std::vector<InputConditions> layers;
};

// One set of output layers sharing the same index
struct OutputSet
{
    OutputSet(const QString &name) : name(name) {}

    QString name;
    // Maps output layers in mRulesMap to their names in mTargetMap
    QHash<const Layer*, QString> layers;
};

struct RuleMapSetup
{
    /**
     * The TileLayer that defines the input and output regions ('regions').
     */
    const TileLayer *mLayerRegions = nullptr;

    /**
     * The TileLayer that defines the input regions ('regions_input').
     */
    const TileLayer *mLayerInputRegions = nullptr;

    /**
     * The TileLayer that defines the output regions ('regions_output').
     */
    const TileLayer *mLayerOutputRegions = nullptr;

    /**
     * Holds different input sets. A rule matches when any of its input sets
     * match.
     */
    std::vector<InputSet> mInputSets;

    /**
     * Holds different output sets. One of the sets is chosen by chance, so
     * randomness is available.
     */
    std::vector<OutputSet> mOutputSets;

    QSet<QString> mInputLayerNames;
    QSet<QString> mOutputTileLayerNames;
    QSet<QString> mOutputObjectGroupNames;
};

struct RuleInputLayer
{
    const TileLayer *targetLayer = nullptr;   // reference to layer in target map
    int posCount = 0;
};

struct RuleInputLayerPos
{
    int x;                          // position relative to match location
    int y;
    int anyCount;                   // any of these cells
    int noneCount;                  // none of these cells
};

/**
 * An efficient structure for matching purposes. Each data structure has a
 * single container, which keeps things packed together in memory.
 */
struct RuleInputSet
{
    QVector<RuleInputLayer> layers;
    QVector<RuleInputLayerPos> positions;
    QVector<Cell> cells;
};

struct Rule
{
    QRegion inputRegion;
    QRegion outputRegion;
    QVector<RuleInputSet> inputSets;
};

struct CompileContext;
struct ApplyContext;

/**
 * A single context is used for running all active AutoMapper instances on a
 * specific target map.
 */
struct TILED_EDITOR_EXPORT AutoMappingContext
{
    AutoMappingContext(MapDocument *mapDocument);

    MapDocument *targetDocument;
    Map *targetMap;

    QVector<SharedTileset> newTilesets;
    QVector<Layer*> newLayers;
    QVector<AddMapObjects::Entry> newMapObjects;
    QHash<Layer*, Properties> changedProperties;

    QHash<QString, const TileLayer*> inputLayers;
    QHash<QString, TileLayer*> outputTileLayers;
    QHash<QString, ObjectGroup*> outputObjectGroups;

    QList<const TileLayer*> touchedTileLayers;  // only used when initially non-empty

    const TileLayer dummy;  // used in case input layers are missing
};

/**
 * This class does all the work for the automapping feature.
 * basically it can do the following:
 * - check the rules map for rules and store them
 * - compare TileLayers (i. e. check if/where a certain rule must be applied)
 * - copy regions of Maps (multiple Layers, the layerlist is a
 *                         lookup-table for matching the Layers)
 */
class TILED_EDITOR_EXPORT AutoMapper : public QObject
{
    Q_OBJECT

public:
    struct Options
    {
        /**
         * Determines whether all tiles in all touched layers should be deleted
         * first.
         */
        bool deleteTiles = false;

        /**
         * Whether rules can match when their input region is partially outside
         * of the map.
         */
        bool matchOutsideMap = false;

        /**
         * If "matchOutsideMap" is true, treat the out-of-bounds tiles as if they
         * were the nearest inbound tile possible
         */
        bool overflowBorder = false;

        /**
         * If "matchOutsideMap" is true, wrap the map in the edges to apply the
         * automapping rules
         */
        bool wrapBorder = false;

        /**
         * Determines whether a rule is allowed to overlap itself.
         */
        bool noOverlappingRules = false;

        /**
         * Determines whether the rules on the map need to be matched in order.
         */
        bool matchInOrder = false;

        /**
         * This variable determines, how many overlapping tiles should be used.
         * The bigger the more area is remapped at an automapping operation.
         * This can lead to higher latency, but provides a better behavior on
         * interactive automapping.
         */
        int autoMappingRadius = 0;
    };

    using GetCell = std::add_pointer_t<const Cell &(int x, int y, const TileLayer &tileLayer)>;

    /**
     * Constructs an AutoMapper.
     *
     * All data structures, which only rely on the rules map are setup
     * here.
     *
     * @param mapDocument: the target map.
     * @param rulesMap: The map containing the automapping rules. The
     *               AutoMapper takes ownership of this map.
     * @param rulesMapFileName: The filepath to the rule map.
     */
    AutoMapper(std::unique_ptr<Map> rulesMap,
               const QString &rulesMapFileName);
    ~AutoMapper() override;

    QString rulesMapFileName() const;

    /**
     * Checks if the passed \a ruleLayerName is used as input layer in this
     * instance of AutoMapper.
     */
    bool ruleLayerNameUsed(const QString &ruleLayerName) const;

    /**
     * This needs to be called directly before the autoMap call.
     * It sets up some data structures which change rapidly, so it is quite
     * painful to keep these data structures up to date all time. (indices of
     * layers of the working map)
     */
    void prepareAutoMap(AutoMappingContext &context);

    /**
     * Here is done all the automapping.
     *
     * When an \a appliedRegion is provided, it is set to the region where
     * rule outputs have been applied.
     */
    void autoMap(const QRegion &where,
                 QRegion *appliedRegion,
                 AutoMappingContext &context);

    /**
     * Contains any errors which occurred while interpreting the rules map.
     */
    QString errorString() const { return mError; }

    /**
     * Contains any warnings which occurred while interpreting the rules map.
     */
    QString warningString() const { return mWarning; }

private:
    /**
     * Reads the map properties of the rulesmap.
     * @return returns true when anything is ok, false when errors occurred.
     */
    bool setupRuleMapProperties();
    void setupInputLayerProperties(InputLayer &inputLayer);

    /**
     * Sets up the layers in the rules map, which are used for automapping.
     * The layers are detected and put in the internal data structures.
     * @return returns true when everything is ok, false when errors occurred.
     */
    bool setupRuleMapLayers();

    /**
     * Searches the rules layer for regions and stores these in \a rules.
     * @return returns true when anything is ok, false when errors occurred.
     */
    bool setupRuleList();

    void setupWorkMapLayers(AutoMappingContext &context) const;
    void compileRule(Rule &rule, const AutoMappingContext &context) const;
    bool compileInputSet(RuleInputSet &index,
                         const InputSet &inputSet,
                         const QRegion &inputRegion, CompileContext &compileContext,
                         const AutoMappingContext &context) const;

    /**
     * This copies all tiles from TileLayer \a srcLayer to TileLayer
     * \a dstLayer.
     *
     * In src the tiles are taken from the \a rect.
     * In dst they get copied to a rectangle given by
     * \a dstX, \a dstY and the size of \a rect.
     * if there is no tile in src TileLayer, there will nothing be copied,
     * so the maybe existing tile in dst will not be overwritten.
     */
    void copyTileRegion(const TileLayer *srcLayer, QRect rect, TileLayer *dstLayer,
                        int dstX, int dstY, const AutoMappingContext &context);

    /**
     * This copies all objects from the \a src_lr ObjectGroup to the \a dst_lr
     * in the given \a rect.
     *
     * The parameter \a dstX and \a dstY offset the copied objects in the
     * destination object group.
     */
    void copyObjectRegion(const ObjectGroup *srcLayer, const QRectF &rect,
                          ObjectGroup *dstLayer, int dstX, int dstY,
                          AutoMappingContext &context);


    /**
     * This copies multiple TileLayers from one map to another.
     * Only the region \a region is considered for copying.
     * In the destination it will come to the region translated by Offset.
     * The parameter \a ruleOutput contains a map of which layers of the rules
     * map should get copied into which layers of the working map.
     */
    void copyMapRegion(const QRegion &region, QPoint Offset,
                       const OutputSet &ruleOutput,
                       AutoMappingContext &context);

    /**
     * This goes through all the positions in \a matchRegion and checks if the
     * \a rule matches there.
     *
     * Calls \a matched for each matching location.
     */
    void matchRule(const Rule &rule,
                   const QRegion &matchRegion,
                   GetCell getCell,
                   const std::function<void (QPoint)> &matched,
                   const AutoMappingContext &context) const;

    /**
     * Applies the given \a rule at each of the given \a positions.
     *
     * Might skip some of the positions to satisfy the NoOverlappingRules
     * option.
     */
    void applyRule(const Rule &rule, QPoint pos, ApplyContext &applyContext,
                   AutoMappingContext &context);

    void addWarning(const QString &text,
                    std::function<void()> callback = std::function<void()>());

    /**
     * map containing the rules, usually different than mTargetMap
     */
    std::unique_ptr<Map> mRulesMap;

    RuleMapSetup mRuleMapSetup;

    /**
     * Stores the input and output region for each rule in mRulesMap, as well
     * as the list of RuleInputSet for each rule.
     */
    std::vector<Rule> mRules;

    /**
     * The name of the processed rules file, used in error reporting.
     */
    QString mRulesMapFileName;

    Options mOptions;

    QString mError;
    QString mWarning;
};

inline QString AutoMapper::rulesMapFileName() const
{
    return mRulesMapFileName;
}

} // namespace Tiled
