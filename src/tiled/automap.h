/*
 * automap.h
 * Copyright 2010, Stefan Beller, stefanbeller@googlemail.com
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

#ifndef AUTOMAP_H
#define AUTOMAP_H

#include <QCoreApplication>
#include <QList>
#include <QPair>
#include <QRegion>
#include <QUndoCommand>
#include <QFile>
#include <QTextStream>

namespace Tiled {

class Layer;
class Map;
class TileLayer;
class Tileset;

namespace Internal {

class MapDocument;

/**
 * This class does all the work for the automapping feature.
 * basically it can do the following:
 * - check the rules map for rules and store them
 * - compare TileLayers (i. e. check if/where a certain rule must be applied)
 * - copy regions of Maps (multiple Layers, the layerlist is a
 *                         lookup-table for matching the Layers)
 */
class AutoMapper
{
    Q_DECLARE_TR_FUNCTIONS(AutoMapper)

public:
    /**
     * Constructs a AutoMapper.
     *
     * @param workingDocument: the map document to work on.
     */
    AutoMapper(MapDocument *workingDocument);
    ~AutoMapper();

    /**
     * Call setupLayers first! (or setupAll which does that for you)
     * @return A list of strings of layers which are likely touched.
     *        these give the Tilelayers which
     *        can be changed in the automap function.
     *        Note: Do not trust they are really there!
     *        If they are specified by rules, but are not needed, they get
     *        deleted again.
     *        But it is sure, that no layer not returned
     *        by this function gets touched.
     */
    QList<QString> getTouchedLayers() const;

    /**
     * Calls all setup-functions in the right order needed for processing
     * a new rules file.
     *
     * param rulePath is only used to have better error message descriptions.
     *
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupRulesMap(Map *rules, QString rulePath);

    void cleanRulesMap();

    /**
     * Sets up the set layer in the mapDocument, which is used for automapping
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupMapDocumentLayers();

    /**
     * Here is done all the automapping.
     */
    void autoMap();

    MapDocument *mapDocument() const { return mMapDocument; }

private:
    /**
     * Searches the rules layer for regions and stores these in \a rules.
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupRuleList();

    /**
     * Sets up the layers in the rules map, which are used for automapping
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupRuleMapLayers();

    /**
     * sets up the tilesets which are used in automapping.
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupTilesets(Map *src, Map *dst);

    /**
     * This copies all Tiles from TileLayer src to TileLayer dst
     *
     * In src the Tiles are taken from the rectangle given by
     * src_x,src_y, width and height.
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
                       const QList< QPair<TileLayer*,TileLayer*> > &LayerTranslation);

    /**
     * This goes through all the positions of the mMapWork and checks if
     * there fits the rule given by the region in mMapRuleSet.
     * if there is a match all Layers are copied to mMapWork.
     * @param rule: the region which should be compared to all positions
     *              of mMapWork
     */
    void applyRule(const QRegion &rule);

    /**
     * This returns whether the given point \a p is part of an existing rule.
     */
    bool isPartOfExistingRule(QPoint p) const;

    /**
     * This creates a rule from a given point.
     * So it will be checked, which regions are coherent to this point
     * and all these regions will be treated as a new rule,
     * which will be returned. To check what is coherent, a
     * breadth first search will be performed, whereas each Tile is a node,
     * and the 4 coherent tiles are connected to this node.
     */
    QRegion createRule(int x, int y) const;

    /**
     * This searches \a map for a layer with the given \a name. Returns that
     * layer if found, and NULL otherwise.
     */
    static TileLayer *findTileLayer(Map *map, const QString &name);

    /**
     * cleans up the data structes filled by setupRuleMapLayers(),
     * so the next rule can be processed.
     */
    void cleanUpRuleMapLayers();

    /**
     * cleans up the data structes filled by setupTilesets(),
     * so the next rule can be processed.
     */
    void cleanUpTilesets();

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
    QList<Tileset*> mAddedTilesets;

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
    QList<TileLayer*> mLayerRuleSets;
    QList<TileLayer*> mLayerRuleNotSets;

    TileLayer *mLayerSet;

    /**
     * List of Regions in mMapRules to know where the rules are
     */
    QList<QRegion> mRules;

    /**
     *  The inner List of Tuples with layers is needed for translating
     * tile layers from mMapRules to mMapWork.
     * the outer list is used to hold different translation tables
     * => one of the inner lists is chosen by chance
     */
    QList<QList<QPair<TileLayer*,TileLayer*> >* > mLayerList;

    /**
     * store the name of the processed rules file, to have detailed
     * error messages available
     */
    QString mRulePath;
};

/**
 * This is a wrapper class for the AutoMapper class.
 * Here in this class only undo/redo functionality for one rulemap
 * is provided.
 * This class' static function handleFile is initially called, when starting
 * an automapping.
 * That static function creates an instance of this class for each map
 * containing rules.
 * This class will take a snapshot of the layers before and after the automapping
 * is done. In between an instance of AutoMapper is doing the work.
 */
class AutomaticMapping : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(AutomaticMapping)
public:
    void undo();
    void redo();

    /**
     * This function parses the "rules.txt" file.
     * For each path which is a rule, (fileextension is tmx) the AutoMapper class
     * is setup properly and an AutomaticMapping object is called to apply
     * the changes of the AutoMapper.
     *
     * If a fileextension is txt, this file will be opened and searched for rules
     * again.
     */
    static void handleFile(MapDocument *mapDocument, const QString &filePath);

private:
    /**
     * constructor and destructor can be private:
     * they are called by the static class function handleFile
     */
    AutomaticMapping(AutoMapper *autoMapper);
    ~AutomaticMapping();

    Layer *swapLayer(int layerIndex, Layer *layer);

    MapDocument *mMapDocument;
    QList<Layer*> mLayersAfter;
    QList<Layer*> mLayersBefore;
};
} // namespace Internal
} // namespace Tiled

#endif // AUTOMAP_H
