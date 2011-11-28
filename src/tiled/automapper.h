/*
 * automapper.h
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

#ifndef AUTOMAPPER_H
#define AUTOMAPPER_H

#include <QList>
#include <QPair>
#include <QRegion>

#include <QSet>
#include <QString>
#include <QVector>

namespace Tiled {

class Map;
class TileLayer;
class Tileset;

namespace Internal {

class MapDocument;

typedef QMap<TileLayer*, int> IndexByTileLayer;

/**
 * This class does all the work for the automapping feature.
 * basically it can do the following:
 * - check the rules map for rules and store them
 * - compare TileLayers (i. e. check if/where a certain rule must be applied)
 * - copy regions of Maps (multiple Layers, the layerlist is a
 *                         lookup-table for matching the Layers)
 */
class AutoMapper : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an AutoMapper.
     *
     * @param workingDocument: the map to work on.
     */
    AutoMapper(MapDocument *workingDocument, QString setlayer);
    ~AutoMapper();

    /**
     * This sets up some internal data structures, which do not change,
     * so it is needed only when loading the rules map.
     */
    bool prepareLoad(Map *rules, const QString &rulePath);

    /**
     * Call prepareLoad first! Returns a set of strings describing the layers,
     * which could be touched considering the given layers of the
     * rule map.
     */
    QSet<QString> getTouchedLayers() const;

    /**
     * This needs to be called directly before the autoMap call.
     * It sets up some data structures which change rapidly, so it is quite
     * painful to keep these datastructures up to date all time. (indices of
     * layers of the working map)
     */
    bool prepareAutoMap();

    /**
     * Here is done all the automapping.
     */
    void autoMap(QRegion *where);

    /**
     * This cleans all datastructures, which are setup via prepareAutoMap,
     * so the auto mapper becomes ready for its next automatic mapping.
     */
    void cleanAll();

    /**
     * Contains all errors until operation was canceled.
     * The errorlist is cleared within prepareLoad and prepareAutoMap.
     */
    QString errorString() const { return mError; }

    /**
     * Contains all warnings which occur at loading a rules map or while
     * automapping.
     * The errorlist is cleared within prepareLoad and prepareAutoMap.
     */
    QString warningString() const { return mWarning; }

private:
    /**
     * Reads the map properties of the rulesmap.
     *
     * param rulePath is only used to have better error message descriptions.
     *
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupRuleMapProperties(Map *rules, const QString &rulePath);

    void cleanUpRulesMap();

    /**
     * Sets up the set layer in the mapDocument, which is used for automapping
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupMapDocumentLayers();

    /**
     * Searches the rules layer for regions and stores these in \a rules.
     * @return returns true when anything is ok, false when errors occured.
     */
    bool setupRuleList();

    /**
     * Sets up the layers in the rules map, which are used for automapping.
     * The layers are detected and put in the internal data structures
     * @return returns true when anything is ok, false when errors occured.
     */
    bool setupRuleMapTileLayers();

    /**
     * Checks if all needed layers in the working map are there.
     * If not, add them in the correct order.
     */
    bool setupMissingLayers();

    /**
     * Checks if the layers setup as in setupRuleMapLayers are still right.
     * If it's not right, correct them.
     * @return returns true if everything went fine. false is returned when
     *         no set layer was found
     */
    bool setupCorrectIndexes();

    /**
     * sets up the tilesets which are used in automapping.
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupTilesets(Map *src, Map *dst);

    /**
     * sets all tiles to 0 in the specified rectangle of the given tile layer.
     */
    void clearRegion(TileLayer *dstLayer, const QRegion &where);

    /**
     * This copies all Tiles from TileLayer src to TileLayer dst
     *
     * In src the Tiles are taken from the rectangle given by
     * src_x, src_y, width and height.
     * In dst they get copied to a rectangle given by
     * dst_x, dst_y, width, height .
     * if there is no tile in src TileLayer, there will nothing be copied,
     * so the maybe existing tile in dst will not be overwritten.
     *
     */
    void copyRegion(TileLayer *src_lr, int src_x, int src_y,
                    int width, int height, TileLayer *dst_lr,
                    int dst_x, int dst_y);

    /**
     * This copies multiple TileLayers from one map to another.
     * To handle multiple Layers, LayerTranslation is a List of Pairs.
     * The first of the QPair is the src and it gets copied to the Layer
     * in second of QPair.
     * Only the region \a region is considered for copying.
     * In the destination it will come to the region translated by Offset.
     */
    void copyMapRegion(const QRegion &region, QPoint Offset,
                       const IndexByTileLayer *LayerTranslation);

    /**
     * This goes through all the positions of the mMapWork and checks if
     * there fits the rule given by the region in mMapRuleSet.
     * if there is a match all Layers are copied to mMapWork.
     * @param rule: the region which should be compared to all positions
     *              of mMapWork
     * @return where: an rectangle where the rule actually got applied
     */
    QRect applyRule(const QRegion &rule, const QRect &where);

    /**
     * Cleans up the data structes filled by setupRuleMapLayers(),
     * so the next rule can be processed.
     */
    void cleanUpRuleMapLayers();

    /**
     * Cleans up the data structes filled by setupTilesets(),
     * so the next rule can be processed.
     */
    void cleanTilesets();

    /**
     * Cleans up the added tile layers setup by setupMissingLayers(),
     * so we have a minimal addition of tile layers by the automapping.
     */
    void cleanTileLayers();

    /**
     * Checks if this the rules from the given rules map could be used anyway
     * by comparing the used tilesets of the set layers and ruleset layer.
     */
    bool setupRulesUsedCheck();

    /**
     * where to work in
     */
    MapDocument *mMapDocument;

    /**
     * the same as mMapDocument->map()
     */
    Map *mMapWork;

    /**
     * map containing the rules, usually different than mMapWork
     */
    Map *mMapRules;

    /**
     * This contains all added tilesets as pointers.
     * if rules use Tilesets which are not in the mMapWork they are added.
     * keep track of them, because we need to delete them afterwards,
     * when they still are unused
     * they will be added while setupTilesets().
     * they will be deleted at Destructor of AutoMapper.
     */
    QVector<Tileset*> mAddedTilesets;

    /**
     * description see: mAddedTilesets, just described by Strings
     */
    QList<QString> mAddedTileLayers;

    /**
     * RuleRegions is the layer where the regions are defined.
     */
    TileLayer *mLayerRuleRegions;

    /**
     * mLayerSet is compared at each tile if it matches any Tile within the
     * mLayerRuleSets list
     * it must not match with any Tile to mLayerRuleNotSets
     */
    QVector<TileLayer*> mLayerRuleSets;
    QVector<TileLayer*> mLayerRuleNotSets;

    /**
     * This stores the name of the layer, which is used in the working map to
     * setup the automapper.
     * Until this variable was introduced it was called "set" (hardcoded)
     */
    QString mSetLayer;

    /**
     * This is the index of the tile layer, which is used in the working map for
     * automapping.
     * So if anything is correct mMapWork->layerAt(mLayerSet)->name()
     * equals mSetLayer.
     */
    int mSetLayerIndex;

    /**
     * List of Regions in mMapRules to know where the rules are
     */
    QList<QRegion> mRules;

    /**
     * The inner set with layers to indexes is needed for translating
     * tile layers from mMapRules to mMapWork.
     *
     * The key is the pointer to the layer in the rulemap. The
     * pointer to the layer within the working map is not hardwired, but the
     * position in the layerlist, where it was found the last time.
     * This loosely bound pointer ensures we will get the right layer, since we
     * need to check before anyway, and it is still fast.
     *
     * The list is used to hold different translation tables
     * => one of the tables is chosen by chance, so randomness is available
     */
    QList<IndexByTileLayer*> mLayerList;

    /**
     * store the name of the processed rules file, to have detailed
     * error messages available
     */
    QString mRulePath;

    /**
     * determines if all tiles in all touched layers should be deleted first.
     */
    bool mDeleteTiles;

    /**
     * This variable determines, how many overlapping tiles should be used.
     * The bigger the more area is remapped at an automapping operation.
     * This can lead to higher latency, but provides a better behavior on
     * interactive automapping.
     * It defaults to zero.
     */
    int mAutoMappingRadius;

    /**
     * Determines if a rule is allowed to overlap itself.
     */
    bool mNoOverlappingRules;

    QList<QString> mTouchedLayers;

    QString mError;

    QString mWarning;
};

} // namespace Internal
} // namespace Tiled

#endif // AUTOMAPPER_H
