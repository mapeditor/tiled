/*
 * automap.cpp
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


#include "addremovelayer.h"
#include "automap.h"
#include "changeproperties.h"
#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilepainter.h"
#include "tileset.h"
#include "undodock.h"

#include <QIODevice>
#include <QList>
#include <QMessageBox>
#include <QPair>
#include <QPoint>
#include <QString>
#include <QUndoStack>


using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
AutoMapper::AutoMapper(MapDocument *workingDocument, Map *rules_map):
mMapDocument(workingDocument),
mMapWork(NULL),
mMapRules(rules_map)
{
    if(mMapDocument)mMapWork = mMapDocument->map();
}

bool AutoMapper::setupLayers(void)
{
    QString mError = QLatin1String("");

    cleanUpLayers();

    mLayerRuleSet     = findTileLayer(mMapRules, QLatin1String("ruleset"));
    mLayerRuleRegions = findTileLayer(mMapRules, QLatin1String("ruleregions"));
    mLayerSet         = findTileLayer(mMapWork, QLatin1String("set"));

    QString prefix = QLatin1String("rule_");
    foreach (Layer *layer, mMapRules->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer())
            if (layer->name().startsWith(prefix,Qt::CaseInsensitive)) {
                QString *name = new QString(layer->name());
                name->remove(0,prefix.length());
                TileLayer *t = findTileLayer(mMapWork,*name);
                if(!t){
                    const int index = mMapWork->layerCount();
                    t = new TileLayer(
                        *name, 0, 0,mMapWork->width(), mMapWork->height()
                        );
                    mMapDocument->undoStack()->push(
                        new AddLayer(mMapDocument, index, t)
                        );
                    mMapDocument->setCurrentLayer(index);
                    mAddedTileLayers.append(t->name());
                }
                mLayerList << new QPair<TileLayer*, TileLayer*>(tileLayer,t);
                delete name;
            }
    }

    if (!mLayerRuleRegions) {
        mError += tr("No ruleRegions layer found!\n");
    }
    if (!mLayerSet) {
        mError += tr("No set layer found!\n");
    }
    if (!mLayerRuleSet) {
        mError += tr("No ruleSet layer found!\n");
    }

    if (mError.compare(QLatin1String(""))){
        QMessageBox msgBox;
        msgBox.setText(mError);
        msgBox.exec();
        return false;
    }

    return true;
}

bool AutoMapper::setupAll(void)
{
    if (!setupLayers())
        return false;

    if (!setupRules())
        return false;

    if (!setupTilesets(mMapRules,mMapWork))
        return false;

    return true;
}

QList<QString> AutoMapper::getTouchedLayers(void)
{
    QList<QString> ret;
    QList< QPair<TileLayer*, TileLayer*>*>::iterator j;

    for (j = mLayerList.begin(); j != mLayerList.end(); ++j){
        ret<<(*j)->second->name();
    }

    return ret;
}

TileLayer *AutoMapper::findTileLayer(Map *map, const QString &whichone)
{
    TileLayer *ret = NULL;
    QString mError = QLatin1String("");

    foreach (Layer *layer, map->layers()) {
        if (layer->name().compare(whichone,
                                  Qt::CaseInsensitive) == 0) {
            if (TileLayer *tileLayer = layer->asTileLayer()) {
                if (ret) {
                    mError = tr("Multiple layers ");
                    mError += whichone;
                    mError +=tr(" found!");
                }
                ret = tileLayer;
            }
        }
    }

    if (mError.compare(QLatin1String(""))) {
        QMessageBox msgBox;
        msgBox.setText(mError);
        msgBox.exec();
    }

    return ret;
}

QRegion *AutoMapper::getExistingRule(QPoint p)
{
    QList<QRegion*>::iterator i;

    for (i = mRules.begin(); i != mRules.end(); ++i)
        if((*i)->contains(p))return (*i);

    return NULL;
}

QRegion *AutoMapper::createRule(int x, int y)
{
    QRegion *ret = new QRegion(x,y,1,1);
    QList<QPoint> addPoints;
    Tile *match = mLayerRuleRegions->tileAt(x,y);
    addPoints.append(QPoint(x,y));

    while(!addPoints.empty()){
        QPoint current = addPoints.first();
        addPoints.removeFirst();
        x = current.x();
        y = current.y();
        if(mLayerRuleRegions->tileAt(x-1,y) == match &&
            (!(*ret).contains(QPoint(x-1,y)))){
                (*ret)+=QRegion(x-1,y,1,1);
                addPoints.append(QPoint(x-1,y));
        }
        if(mLayerRuleRegions->tileAt(x+1,y) == match &&
            (!(*ret).contains(QPoint(x+1,y)))){
                (*ret)+=QRegion(x+1,y,1,1);
                addPoints.append(QPoint(x+1,y));
        }
        if(mLayerRuleRegions->tileAt(x,y-1) == match &&
            (!(*ret).contains(QPoint(x,y-1)))){
                (*ret)+=QRegion(x,y-1,1,1);
                addPoints.append(QPoint(x,y-1));
        }
        if(mLayerRuleRegions->tileAt(x,y+1) == match &&
            (!(*ret).contains(QPoint(x,y+1)))){
                (*ret)+=QRegion(x,y+1,1,1);
                addPoints.append(QPoint(x,y+1));
        }
    }

    return ret;
}

bool AutoMapper::setupRules(void)
{
    int x;
    int y;
    cleanUpRuleRegions();

    for(y=1; y < mMapRules->height(); y++ ){
        for(x=1; x < mMapRules->width();  x++ ){
            if( mLayerRuleRegions->tileAt(x,y) &&
                getExistingRule(QPoint(x,y))==NULL)
                    mRules<<createRule(x,y);
        }
    }
    return true;
}


static Tileset *findSimilarTileset(Tileset *tileset,
                                   const QList<Tileset*> &tilesets)
{
    foreach (Tileset *candidate, tilesets) {
        if (candidate != tileset
            && candidate->imageSource() == tileset->imageSource()
            && candidate->tileWidth() == tileset->tileWidth()
            && candidate->tileHeight() == tileset->tileHeight()
            && candidate->tileSpacing() == tileset->tileSpacing()
            && candidate->margin() == tileset->margin()){
                return candidate;
        }
    }
    return 0;
}

bool AutoMapper::setupTilesets(Map *src, Map *dst)
{
    QList<Tileset*> existingTilesets = dst->tilesets();
    // Add tilesets that are not yet part of dst map
    foreach (Tileset *tileset, src->tilesets()) {
        if (existingTilesets.contains(tileset))
            continue;

        Tileset *replacement = findSimilarTileset(tileset, existingTilesets);
        if (!replacement) {
            mAddedTilesets.append(tileset);
            mMapDocument->undoStack()->push(
                                new AddTileset(mMapDocument, tileset));
            continue;
        }

        // Merge the tile properties
        const int sharedTileCount = qMin(tileset->tileCount(),
                                         replacement->tileCount());
        for (int i = 0; i < sharedTileCount; ++i) {
            Tile *replacementTile = replacement->tileAt(i);
            Properties properties = replacementTile->properties();
            properties.merge(tileset->tileAt(i)->properties());
            mMapDocument->undoStack()->push(new ChangeProperties(tr("Tile"),
                                                     replacementTile,
                                                     properties));
        }
        src->replaceTileset(tileset, replacement);
        delete tileset;
    }
    return true;
}

void AutoMapper::copyRegion(TileLayer *src_lr, \
                            int src_x, int src_y, \
                            int width, int height, \
                            TileLayer *dst_lr, \
                            int dst_x, int dst_y)
{
    TilePainter *tp = new TilePainter(mMapDocument, dst_lr);
    Tile *t;
    for(int x=0;x<width;x++){
        for(int y=0;y<height;y++){
            t = src_lr->tileAt( src_x + x, src_y + y );
            if(t){
                tp->setTile( dst_x + x, dst_y + y, t );
            }
        }
    }
    delete tp;
}


void AutoMapper::copyMapRegion(
                    const QRegion &region, const QPoint Offset, \
                    QList< QPair<TileLayer*,TileLayer*> *> *LayerTranslation)
{
    QPair<TileLayer*,TileLayer*> *lr;
    QList< QPair<TileLayer*,TileLayer*>*>::iterator lr_i;
    for(lr_i = LayerTranslation->begin();
        lr_i != LayerTranslation->end();
        ++lr_i){
        lr = *lr_i;
        foreach(QRect rect, region.rects()){
            copyRegion(lr->first,rect.x(),rect.y(), \
                       rect.width(),rect.height(), \
                       lr->second,rect.x()+Offset.x(),rect.y()+Offset.y()
                       );
        }
    }
}


bool AutoMapper::compareLayers(TileLayer *l1, TileLayer *l2, \
                               const QRegion &r1, QPoint Offset)
{

    Tile *t1;
    Tile *t2;
    foreach(QRect rect, r1.rects()){
        for(int x = rect.left(); x <= rect.right(); x++){
            for(int y = rect.top(); y <= rect.bottom(); y++){
                if(! (l1->contains(x,y) &&
                      l2->contains(x + Offset.x(),y + Offset.y())))
                    return false;
                t1=l1->tileAt(x,y);
                t2=l2->tileAt(x + Offset.x(),y + Offset.y());
                if(t1 && t2 && t1!=t2)
                    return false;
            }
        }
    }
    return true;
}



void AutoMapper::ApplyRule(const QRegion &rule)
{
    int x = - rule.boundingRect().left();
    int y = - rule.boundingRect().top();
    const int max_x = x + mMapWork->width();
    const int max_y = y + mMapWork->height();

    while( y <= max_y){
        while( x <= max_x){
            if (compareLayers(mLayerRuleSet,mLayerSet,rule,QPoint(x,y)))
                copyMapRegion(rule,QPoint(x,y),&mLayerList );
            x++;
        }
        x = - rule.boundingRect().left();
        y++;
    }
}

void AutoMapper::automap(void)
{
    foreach (QRegion *rule, mRules)
        ApplyRule(*rule);
}

void AutoMapper::cleanUpLayers(void)
{
    foreach(QString t, mAddedTileLayers){
        const int layerindex = mMapWork->indexOfLayer(t);
        if(layerindex != -1){
            TileLayer *t = mMapWork->layerAt(layerindex)->asTileLayer();
            if(t->isEmpty()){
                mMapDocument->undoStack()->push(
                            new RemoveLayer(mMapDocument,layerindex));
            }
        }
    }
    QList< QPair<TileLayer*, TileLayer*>*>::iterator j;
    for (j = mLayerList.begin(); j != mLayerList.end(); ++j)
        delete *j;
}

void AutoMapper::cleanUpRuleRegions(void)
{
    QList<QRegion*>::iterator i;
    for (i = mRules.begin(); i != mRules.end(); ++i)
        delete (*i);
}

void AutoMapper::cleanUpTilesets(void)
{
    QUndoCommand *cmd;
    //check only those this class added:
    foreach(Tileset *t, mAddedTilesets)
        if(!mMapWork->isTilesetUsed(t)){
            const int layerindex = mMapWork->indexOfTileset(t);
            if(layerindex != -1){
                cmd = new RemoveTileset(mMapDocument,layerindex, t);
                mMapDocument->undoStack()->push(cmd);
            }
        }
}

AutoMapper::~AutoMapper(void)
{
    cleanUpRuleRegions();
    cleanUpLayers();
    cleanUpTilesets();
}


AutomaticMapping::AutomaticMapping(MapDocument *workingDocument, Map *rules):
mMapDocument(workingDocument)
{
    AutoMapper *a = new AutoMapper(workingDocument,rules);
    if(a->setupAll()){
        Map *map = mMapDocument->map();
        foreach(QString layername, a->getTouchedLayers()){
            const int layerindex = map->indexOfLayer(layername);
            mLayers_prev<<map->layerAt(layerindex)->clone();
        }
        a->automap();
        foreach(QString layername, a->getTouchedLayers()){
            const int layerindex = map->indexOfLayer(layername);
            //layerindex exists,because AutoMapper is still alive, dont check
            mLayers_after<<map->layerAt(layerindex)->clone();
        }
    }
    delete a;
}

AutomaticMapping::~AutomaticMapping(void)
{
    QList<Layer*>::iterator i;
    for(i = mLayers_after.begin(); i != mLayers_after.end();++i)
        delete *i;
    for(i = mLayers_prev.begin(); i != mLayers_prev.end();++i)
        delete *i;
}

void AutomaticMapping::undo(void)
{
    Map *map = mMapDocument->map();
    QList<Layer*>::iterator i;
    for(i = mLayers_prev.begin();i!=mLayers_prev.end();++i){
        const int layerindex = map->indexOfLayer((*i)->name());
        if(layerindex != -1)
            //just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex,(*i)->clone());
    }
}
void AutomaticMapping::redo(void){
    Map *map = mMapDocument->map();
    QList<Layer*>::iterator i;
    for(i = mLayers_after.begin();i!=mLayers_after.end();++i){
        const int layerindex = (map->indexOfLayer((*i)->name()));
        if(layerindex != -1)
            //just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex,(*i)->clone());
    }
}

Layer *AutomaticMapping::swapLayer(const int layerIndex, Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayer();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(layerIndex);
    layerModel->insertLayer(layerIndex, layer);

    if (layerIndex == currentIndex)
        mMapDocument->setCurrentLayer(layerIndex);

    return replaced;
}

}//namespace Tiled
