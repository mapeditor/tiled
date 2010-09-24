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

#ifndef AUTO_MAP_H
#define AUTO_MAP_H

#include "map.h"
#include "tilelayer.h"
#include "tileset.h"
#include "addremovetileset.h"

#include <QList>
#include <QPair>
#include <QPoint>
#include <QString>


using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {

/**
 *This is a wrapper class for the AutoMapper class.
 *Here in this class only Redo/Undo functionality is provided.
 */
class AutomaticMapping: QObject, public QUndoCommand{
public:
    AutomaticMapping(MapDocument *workingDocument, Map *rules);
    ~AutomaticMapping(void);

    void undo(void);
    void redo(void);

private:
    Layer *swapLayer(int layerIndex, Layer *layer);

    /*variables*/
    MapDocument *mMapDocument;
    QList<Layer*> mLayers_after;
    QList<Layer*> mLayers_prev;
};

/**
 *This class does all the work for the automapping feature.
 *basically it can do the following:
 * - check the rules map for rules and store them
 * - compare TileLayers (i. e. check if/where a certain rule must be applied)
 * - copy regions of Maps (multiple Layers, the layerlist is a
 *                         lookup-table for matching the Layers)
 */

class AutoMapper: QObject{
public:
    /**
     *Constructs a AutoMapper.
     *
     *@param workingDocument: the map document to work on.
     *@param rules: the map in which the ruleset is defined.
     */
    AutoMapper(MapDocument *workingDocument, Map *rules);
    ~AutoMapper(void);

    /**
     *
     * call setupLayers first! (or setupAll which does that for you)
     * @param void
     * @return a list of Strings of Layers which are likely touched.
     *        these give the Tilelayers which
     *        can be changed in the automap function.
     *        Note: Do not trust they are really there!
     *        If they are specified by rules, but are not needed, they get
     *        deleted again.
     *        But it is sure, that no layer not returned
     *        by this function gets touched.
     */
    QList<QString> getTouchedLayers(void);

    /**
     * calls all setupXXX functions in the right order.
     * @param void
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupAll(void);

    /**
     * here is done all the automapping.
     * @param void
     * @return void
     */
    void automap(void);

private:
    /**
     * searches the rules layer for regions and stores these in \a rules.
     * @param void
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupRules(void);

    /**
     * sets up the Layers, which are used for automapping
     * @param void
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupLayers(void);

    /**
     * sets up the Tilesets which are used in automapping.
     * @return returns true when anything is ok, false when errors occured.
     *        (in that case will be a msg box anyway)
     */
    bool setupTilesets(Map *src, Map *dst);

    /**
     * This compares two TileLayers.
     * The first TileLayer is examined at Qregion r1
     * The second TileLayer is examined at r1+Offset
     * Only when at both places are Tiles, these will be compared.
     * if there is no tile at all in one layer, it is treated as equal
     *
     * @return bool, if the TileLayers are the same at the specific regions
     */
    bool compareLayers(TileLayer *l1, TileLayer *l2, \
                       const QRegion &r1, QPoint Offset);
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
    void copyRegion(TileLayer *src_lr, int src_x, int src_y, \
                    int width, int height, TileLayer *dst_lr, \
                    int dst_x, int dst_y);
    /**
     * This copies multiple TileLayers from one map to another.
     * To handle multiple Layers, LayerTranslation is a List of Pairs.
     * The first of the QPair is the src and it gets copied to the Layer
     * in second of QPair.
     * Only the region \a region is considered for copying.
     * In the destination it will come to the region translated by Offset.
     */
    void copyMapRegion(const QRegion &region, QPoint Offset, \
                       QList< QPair<TileLayer*,TileLayer*>*> *LayerTranslation);
    /**
     * This goes through all the positions of the mMapWork and checks if
     * there fits the rule given by the region in mMapRuleSet.
     * if there is a match all Layers are copied to mMapWork.
     * @param rule: the region which should be compared to all positions
     *              of mMapWork
     */
    void ApplyRule(const QRegion &rule);

    /**
     * This returns the rule which contains the given point p.
     * If there is no such rule, NULL will be returned.
     * @param   p a point in mMapRuleSet, to be checked.
     * @return  the matching rule given by a QRegion,
     *          NULL if there is no such rule.
     */
    QRegion *getExistingRule(QPoint p);

    /**
     * This creates a Rule from a given Point.
     * So it will be checked, which Regions are coherent to this point
     * and all these regions will be treated as a new rule,
     * which will be returned. To check what is coherent, a
     * breadth first search will be performed, whereas each Tile is a node,
     * and the 4 coherent Tiles are connected to this node.
     */
    QRegion *createRule(int x, int y);

    /**
     * This searches a given Map, if it contains the TileLayer given
     * by QString whichone.
     * if there is no such Layer, NULL will be returned.
     */
    TileLayer *findTileLayer(Map *map, const QString &whichone);

/*for cleanup: delete all the objects created with 'new'*/
    void cleanUpLayers();
    void cleanUpRuleRegions();
    void cleanUpTilesets();
/*variables*/
    //where to work in
    MapDocument *mMapDocument;
    //the same as mMapDocument->map()
    Map *mMapWork;
    //map containing the rules, usually different that mMapWork
    Map *mMapRules;
    //This contains all added tilesets as pointers.
    //if rules use Tilesets which are not in the mMapWork they are added.
    //keep track of them, because we need to delete them afterwards,
    //when they still are unused
    //they will be added while setupTilesets().
    //they will be deleted at Destructor of AutoMapper.
    QList<Tileset*> mAddedTilesets;
    //description see: mAddedTilesets, just described by Strings
    QList<QString> mAddedTileLayers;

    //RuleRegions is the layer where the regions are defined.
    TileLayer *mLayerRuleRegions;
    //RuleSet and Set are compared to check where to apply rules.
    TileLayer *mLayerRuleSet;
    TileLayer *mLayerSet;

    //List of Regions in mMapRules to know where the rules are
    QList<QRegion*> mRules;
    //List of Tuples with layers, these pairs are needed for translating
    //mMapRules to mMapWork.
    QList<QPair<TileLayer*,TileLayer*>*> mLayerList;
};



}//end using Tiled

#endif
