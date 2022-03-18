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

#include "tilededitor_global.h"
#include "tileset.h"

#include <QList>
#include <QMap>
#include <QRegion>
#include <QSet>
#include <QString>
#include <QVector>

#include <memory>
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
    const TileLayer *layer = nullptr;   // reference to layer in target map
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

    /**
     * All input layer names.
     */
    QSet<QString> mInputLayerNames;
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
         * Determines if all tiles in all touched layers should be deleted first.
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
         * Determines if a rule is allowed to overlap itself.
         */
        bool noOverlappingRules = false;

        /**
         * This variable determines, how many overlapping tiles should be used.
         * The bigger the more area is remapped at an automapping operation.
         * This can lead to higher latency, but provides a better behavior on
         * interactive automapping.
         */
        int autoMappingRadius = 0;
    };

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
    AutoMapper(MapDocument *mapDocument,
               std::unique_ptr<Map> rulesMap,
               const QString &rulesMapFileName);
    ~AutoMapper() override;

    /**
     * Checks if the passed \a ruleLayerName is used as input layer in this
     * instance of AutoMapper.
     */
    bool ruleLayerNameUsed(const QString &ruleLayerName) const;

    /**
     * Returns a map of name to target layer, which could be touched
     * considering the output layers of the rule map.
     */
    const QHash<QString, TileLayer *> &touchedTileLayers() const;

    /**
     * This needs to be called directly before the autoMap call.
     * It sets up some data structures which change rapidly, so it is quite
     * painful to keep these data structures up to date all time. (indices of
     * layers of the working map)
     */
    void prepareAutoMap();

    /**
     * Here is done all the automapping.
     */
    void autoMap(const QRegion &where, QRegion *appliedRegion);

    /**
     * This cleans all data structures, which are setup via prepareAutoMap,
     * so the auto mapper becomes ready for its next automatic mapping.
     */
    void finalizeAutoMap();

    /**
     * Contains any errors which occurred while interpreting the rules map.
     */
    QString errorString() const { return mError; }

    /**
     * Contains any warnings which occurred while interpreting the rules map.
     */
    QString warningString() const { return mWarning; }

private:
    struct RuleRegion
    {
        QRegion input;
        QRegion output;
    };

    /**
     * Reads the map properties of the rulesmap.
     * @return returns true when anything is ok, false when errors occurred.
     */
    bool setupRuleMapProperties();
    void setupInputLayerProperties(InputLayer &inputLayer);

    /**
     * Searches the rules layer for regions and stores these in \a rules.
     * @return returns true when anything is ok, false when errors occurred.
     */
    bool setupRuleList();

    /**
     * Sets up the layers in the rules map, which are used for automapping.
     * The layers are detected and put in the internal data structures.
     * @return returns true when everything is ok, false when errors occurred.
     */
    bool setupRuleMapLayers();

    void setupWorkMapLayers();
    void setupTilesets();

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
                        int dstX, int dstY);

    /**
     * This copies all objects from the \a src_lr ObjectGroup to the \a dst_lr
     * in the given \a rect.
     *
     * The parameter \a dstX and \a dstY offset the copied objects in the
     * destination object group.
     */
    void copyObjectRegion(const ObjectGroup *srcLayer, const QRectF &rect,
                          ObjectGroup *dstLayer, int dstX, int dstY);


    /**
     * This copies multiple TileLayers from one map to another.
     * Only the region \a region is considered for copying.
     * In the destination it will come to the region translated by Offset.
     * The parameter \a ruleOutput contains a map of which layers of the rules
     * map should get copied into which layers of the working map.
     */
    void copyMapRegion(const QRegion &region, QPoint Offset,
                       const OutputSet &ruleOutput);

    /**
     * This goes through all the positions in \a applyRegion and checks if the
     * rule given by \a ruleRegion matches there.
     *
     * If there is a match, all output layers are copied to mTargetMap.
     *
     * When an \a appliedRegion is provided, it is set to the region where
     * rule outputs have been applied.
     */
    void applyRule(const RuleRegion &ruleRegion,
                   const QRegion &applyRegion,
                   QRegion *appliedRegion = nullptr);

    /**
     * Cleans up the data structures filled by setupTilesets(),
     * so the next rule can be processed.
     */
    void cleanTilesets();

    /**
     * Cleans up the added tile layers setup by setupMissingLayers(),
     * so we have a minimal addition of tile layers by the automapping.
     */
    void cleanEmptyLayers();

    void addWarning(const QString &text,
                    std::function<void()> callback = std::function<void()>());

    /**
     * where to work in
     */
    MapDocument *mTargetDocument;

    /**
     * the same as mMapDocument->map()
     */
    Map *mTargetMap;

    /**
     * map containing the rules, usually different than mTargetMap
     */
    std::unique_ptr<Map> mRulesMap;

    /**
     * Contains the tilesets that have been added to mTargetMap.
     *
     * Any tilesets used by the rules map, which are not in the mTargetMap, are
     * added before applying the rules. Afterwards, we remove the added
     * tilesets that didn't end up getting used.
     *
     * @see setupTilesets()
     * @see cleanTilesets()
     */
    QVector<SharedTileset> mAddedTilesets;

    /**
     * Contains the layers that have been added to mTargetMap.
     *
     * Any output layers that were not found in mTargetMap are added. If they
     * remain empty after applying the rules, they are removed again.
     *
     * @see setupMissingLayers()
     * @see cleanEmptyLayers()
     */
    QVector<Layer*> mAddedLayers;

    RuleMapSetup mRuleMapSetup;

    /**
     * Stores the input and output region for each rule in mRulesMap.
     */
    QVector<RuleRegion> mRuleRegions;

    /**
     * The name of the processed rules file, used in error reporting.
     */
    QString mRulesMapFileName;

    Options mOptions;

    /*
     * These map layer names to the target layers in mTargetMap. To ensure no
     * roaming pointers are used, this mapping is updated right before each
     * automapping.
     *
     * @see setupWorkMapLayers()
     */
    QHash<QString, const TileLayer*> mSetLayers;
    QHash<QString, TileLayer*> mTouchedTileLayers;
    QHash<QString, ObjectGroup*> mTouchedObjectGroups;

    QString mError;
    QString mWarning;
};

inline const QHash<QString, TileLayer*> &AutoMapper::touchedTileLayers() const
{
    return mTouchedTileLayers;
}

} // namespace Tiled
