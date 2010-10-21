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

#include "automap.h"

#include "addremovelayer.h"
#include "addremovetileset.h"
#include "changeproperties.h"
#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilepainter.h"
#include "tileset.h"
#include "tmxmapreader.h"

#include <QMessageBox>
#include <QUndoStack>
#include <QFileInfo>

using namespace Tiled;
using namespace Tiled::Internal;

AutoMapper::AutoMapper(MapDocument *workingDocument)
    : mMapDocument(workingDocument),
      mMapWork(workingDocument->map()),
      mMapRules(0)
{
}

bool AutoMapper::setupMapDocumentLayers()
{
    mLayerSet         = findTileLayer(mMapWork, QLatin1String("set"));

    if (!mLayerSet) {
        QString error = tr("No set layer found!");
        QMessageBox msgBox;
        msgBox.setText(error);
        msgBox.exec();
        return false;
    }

    return true;
}

bool AutoMapper::setupRuleMapLayers()
{
    cleanUpRuleMapLayers();
    cleanUpTilesets();

    mLayerSet         = findTileLayer(mMapWork, QLatin1String("set"));
    mLayerRuleRegions = findTileLayer(mMapRules, QLatin1String("ruleregions"));

    // allow multiple Set Layers
    foreach (Layer *layer, mMapRules->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            if (tileLayer->name().startsWith(
                    QLatin1String("ruleset"), Qt::CaseInsensitive)) {
                mLayerRuleSets.append(tileLayer);
            }
        }
    }

    // allow multiple ruleNotSet Layers
    foreach (Layer *layer, mMapRules->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            if (tileLayer->name().startsWith(
                    QLatin1String("rulenotset"), Qt::CaseInsensitive)) {
                mLayerRuleNotSets.append(tileLayer);
            }
        }
    }

    QString prefix = QLatin1String("rule_");
    foreach (Layer *layer, mMapRules->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            if (layer->name().startsWith(prefix, Qt::CaseInsensitive)) {
                QString name = layer->name();
                name.remove(0, prefix.length());
                TileLayer *t = findTileLayer(mMapWork, name);
                if (!t) {
                    const int index = mMapWork->layerCount();
                    t = new TileLayer(name, 0, 0,
                                      mMapWork->width(), mMapWork->height());
                    mMapDocument->undoStack()->push(
                                new AddLayer(mMapDocument, index, t));
                    mMapDocument->setCurrentLayer(index);
                    mAddedTileLayers.append(t->name());
                }
                mLayerList << QPair<TileLayer*, TileLayer*>(tileLayer, t);
            }
        }
    }

    QString error;

    if (!mLayerRuleRegions)
        error += tr("No ruleRegions layer found!") + QLatin1Char('\n');

    if (!mLayerSet)
        error += tr("No set layers found!") + QLatin1Char('\n');

    if (mLayerRuleSets.size() == 0)
        error += tr("No ruleSet layer found!") + QLatin1Char('\n');

    // no need to check for mLayerRuleNotSets.size() == 0 here.
    // these layers are not necessary.

    if (!error.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(error);
        msgBox.exec();
        return false;
    }

    return true;
}

bool AutoMapper::setupRulesMap(Map *rules)
{
    mMapRules = rules;

    if (!setupRuleMapLayers())
        return false;

    if (!setupRuleList())
        return false;

    if (!setupTilesets(mMapRules, mMapWork))
        return false;

    return true;
}

void AutoMapper::cleanRulesMap()
{
    mMapRules = 0;

    cleanUpRuleMapLayers();
    cleanUpTilesets();
    mRules.clear();
}

QList<QString> AutoMapper::getTouchedLayers() const
{
    QList<QString> ret;
    QList<QPair<TileLayer*, TileLayer*> >::const_iterator j;

    for (j = mLayerList.constBegin(); j != mLayerList.constEnd(); ++j)
        ret << j->second->name();

    return ret;
}

TileLayer *AutoMapper::findTileLayer(Map *map, const QString &name)
{
    TileLayer *ret = 0;
    QString error;

    foreach (Layer *layer, map->layers()) {
        if (layer->name().compare(name, Qt::CaseInsensitive) == 0) {
            if (TileLayer *tileLayer = layer->asTileLayer()) {
                if (ret)
                    error = tr("Multiple layers %1 found!").arg(name);
                ret = tileLayer;
            }
        }
    }

    if (!error.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(error);
        msgBox.exec();
    }

    return ret;
}

bool AutoMapper::isPartOfExistingRule(QPoint p) const
{
    foreach (const QRegion &region, mRules)
        if (region.contains(p))
            return true;

    return false;
}

QRegion AutoMapper::createRule(int x, int y) const
{
    QRegion ret(x, y, 1, 1);
    QList<QPoint> addPoints;
    Tile *match = mLayerRuleRegions->tileAt(x, y);
    addPoints.append(QPoint(x, y));

    while (!addPoints.empty()) {
        const QPoint current = addPoints.takeFirst();
        x = current.x();
        y = current.y();
        if (mLayerRuleRegions->tileAt(x - 1, y) == match
                && !ret.contains(QPoint(x - 1, y))) {
            ret += QRegion(x - 1, y, 1, 1);
            addPoints.append(QPoint(x - 1, y));
        }
        if (mLayerRuleRegions->tileAt(x + 1, y) == match
                && !ret.contains(QPoint(x + 1, y))) {
            ret += QRegion(x + 1, y, 1, 1);
            addPoints.append(QPoint(x + 1, y));
        }
        if (mLayerRuleRegions->tileAt(x, y - 1) == match
                && !ret.contains(QPoint(x, y - 1))) {
            ret += QRegion(x, y - 1, 1, 1);
            addPoints.append(QPoint(x, y - 1));
        }
        if (mLayerRuleRegions->tileAt(x, y + 1) == match
                && !ret.contains(QPoint(x, y + 1))) {
            ret += QRegion(x, y + 1, 1, 1);
            addPoints.append(QPoint(x, y + 1));
        }
    }

    return ret;
}

bool AutoMapper::setupRuleList()
{
    mRules.clear();

    for (int y = 1; y < mMapRules->height(); y++ ) {
        for (int x = 1; x < mMapRules->width();  x++ ) {
            if (mLayerRuleRegions->tileAt(x, y)
                    && !isPartOfExistingRule(QPoint(x, y))) {
                mRules << createRule(x, y);
            }
        }
    }
    return true;
}


static Tileset *findSimilarTileset(const Tileset *tileset,
                                   const QList<Tileset*> &tilesets)
{
    foreach (Tileset *candidate, tilesets) {
        if (candidate != tileset
            && candidate->imageSource() == tileset->imageSource()
            && candidate->tileWidth() == tileset->tileWidth()
            && candidate->tileHeight() == tileset->tileHeight()
            && candidate->tileSpacing() == tileset->tileSpacing()
            && candidate->margin() == tileset->margin()) {
                return candidate;
        }
    }
    return 0;
}

/**
 * this cannot just be replaced by MapDocument::unifyTileset(Map),
 * because here mAddedTileset is modified
 */
bool AutoMapper::setupTilesets(Map *src, Map *dst)
{
    QList<Tileset*> existingTilesets = dst->tilesets();

    // Add tilesets that are not yet part of dst map
    foreach (Tileset *tileset, src->tilesets()) {
        if (existingTilesets.contains(tileset))
            continue;

        QUndoStack *undoStack = mMapDocument->undoStack();

        Tileset *replacement = findSimilarTileset(tileset, existingTilesets);
        if (!replacement) {
            mAddedTilesets.append(tileset);
            undoStack->push(new AddTileset(mMapDocument, tileset));
            continue;
        }

        // Merge the tile properties
        const int sharedTileCount = qMin(tileset->tileCount(),
                                         replacement->tileCount());
        for (int i = 0; i < sharedTileCount; ++i) {
            Tile *replacementTile = replacement->tileAt(i);
            Properties properties = replacementTile->properties();
            properties.merge(tileset->tileAt(i)->properties());

            undoStack->push(new ChangeProperties(tr("Tile"),
                                                 replacementTile,
                                                 properties));
        }
        src->replaceTileset(tileset, replacement);
        delete tileset;
    }
    return true;
}

void AutoMapper::copyRegion(TileLayer *srcLayer, int srcX, int srcY,
                            int width, int height,
                            TileLayer *dstLayer, int dstX, int dstY)
{
    TilePainter tp(mMapDocument, dstLayer);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (Tile *t = srcLayer->tileAt(srcX + x, srcY + y))
                tp.setTile(dstX + x, dstY + y, t);
        }
    }
}

void AutoMapper::copyMapRegion(const QRegion &region, QPoint offset,
                               const QList< QPair<TileLayer*,TileLayer*> > &layerTranslation)
{
    QList< QPair<TileLayer*, TileLayer*> >::const_iterator lr_i;
    for (lr_i = layerTranslation.begin();
         lr_i != layerTranslation.end();
         ++lr_i) {
        foreach (QRect rect, region.rects()) {
            copyRegion(lr_i->first, rect.x(), rect.y(),
                       rect.width(), rect.height(),
                       lr_i->second, rect.x() + offset.x(), rect.y() + offset.y());
        }
    }
}

/**
 * returns a list of all tiles which can be found within all tile layers
 * within the given region.
 */
static QList<Tile*> tilesInRegion(QList<TileLayer*> list, const QRegion &r)
{
    QList<Tile*> tiles;
    foreach (TileLayer *l, list) {
        foreach (QRect rect, r.rects()) {
            for (int x = rect.left(); x <= rect.right(); x++) {
                for (int y = rect.top(); y <= rect.bottom(); y++) {
                    Tile *t = l->tileAt(x, y);
                    if (!tiles.contains(t))
                        tiles.append(t);
                }
            }
        }
    }
    return tiles;
}

/**
 * This function is one of the core functions for understanding the
 * automapping.
 * In this function a certain region (of the "set" layer) is compared to
 * several other layers (ruleSet and ruleNotSet).
 * This Comparision will determine if a rule of automapping matches,
 * so if this rule is applied at this region given
 * by a QRegion and Offset given by a QPoint.
 *
 * This compares the TileLayer l1 ("set" layer) to several others given
 * in the QList listYes (ruleSet) and OList listNo (ruleNotSet).
 * The TileLayer l1 is examined at QRegion r1 + offset
 * The Tilelayers within listYes and listNo are examined at QRegion r1.
 *
 * Basically all matches between l1 and a layer of listYes are considered
 * good, while all matches between l1 and listNo are considered bad.
 *
 * The comparison is done for each position within the QRegion r1.
 * If all positions of the region are considered "good" return true.
 *
 * Now there are several cases to distinguish:
 * - l1 is 0:
 *      obviously there should be no automapping.
 *      So here no rule should be applied. return false
 * - l1 is not 0:
 *      - both listYes and listNo are empty:
 *          should not happen, because with that configuration, absolutly
 *          no condition is given.
 *          return false, assuming this is an errornous rule being applied
 *
 *      - both listYes and listNo are not empty:
 *          When comparing a tile at a certain position of TileLayer l1
 *          to all available Tiles in listYes, there must be at least
 *          one layer, in which there is a match of tiles of l1 and listYes
 *          to consider this position good.
 *          In listNo there must not be a match to consider this position
 *          good.
 *          if there are no tiles within all Layers of one list, all tiles in l1
 *          are considered good, while inspecting this list.
 *
 *
 *      - either of both lists is empty
 *          When comparing a certain position of TileLayer l1 to all Tiles
 *          at the corresponding position this can happen:
 *          A tile of l1 matches a tile of a layer in the list. Then this
 *          is considered as good, if the layer is from the listYes.
 *          Otherwise it is considered bad.
 *
 *          Exception, when having only the listYes:
 *          if at the examined position there are no tiles within all Layers
 *          of the listYes, all tiles except all used tiles within
 *          the layers of that list are considered good.
 *
 *          This exception was added to have a better functionality
 *          (need of less layers.)
 *          It was not added to the case, when having only listNo layers to
 *          avoid total symmetrie between those lists.
 *
 * If all positions are considered good, return true.
 * return false otherwise.
 *
 * @return bool, if the tile layer matches the given list of layers.
 */
static bool compareLayerTo(TileLayer *l1, QList<TileLayer*> listYes,
            QList<TileLayer*> listNo, const QRegion &r1, QPoint offset)
{
    if (listYes.size() == 0 && listNo.size() == 0)
        return false;

    QList<Tile*> tiles;
    if (listYes.size() == 0)
        tiles = tilesInRegion(listNo, r1);
    if (listNo.size() == 0)
        tiles = tilesInRegion(listYes, r1);

    foreach (QRect rect, r1.rects()) {
        for (int x = rect.left(); x <= rect.right(); x++) {
            for (int y = rect.top(); y <= rect.bottom(); y++) {
                // this is only used in the case where only one list has layers
                // it is needed for the exception mentioned above
                bool ruleDefinedListYes = false;
                bool ruleDefinedListNo  = false;

                bool matchListYes = false;
                bool matchListNo  = false;

                if (!l1->contains(x + offset.x(), y + offset.y()))
                    return false;

                Tile *t1 = l1->tileAt(x + offset.x(), y + offset.y());

                // when there is no tile in l1 (= "set" layer),
                // there should be no rule at all
                if (!t1)
                    return false;

                // ruleDefined will be set when there is a tile in at least
                // one layer. if there is a tile in at least one layer, only
                // the given tiles in the different listYes layers are valid.
                // if there is given no tile at all in the listYes layers,
                // consider all tiles valid.

                foreach(TileLayer *l2, listYes) {

                    if (!l2->contains(x, y))
                        return false;

                    Tile *t2 = l2->tileAt(x, y);
                    if (t2)
                        ruleDefinedListYes = true;

                    if (t2 && t1 == t2)
                        matchListYes = true;
                }
                foreach(TileLayer *l2, listNo) {

                    if (!l2->contains(x, y))
                        return false;

                    Tile *t2 = l2->tileAt(x, y);
                    if (t2)
                        ruleDefinedListNo = true;

                    if (t2 && t1 == t2)
                        matchListNo = true;
                }

                // when there are only layers in the listNo
                // check only if these layers are unmatched
                // no need to check explicitly the exception in this case.
                // compiler suggests explicit braces to avoid ambiguous ‘else’
                if (listYes.size() == 0 ) {
                    if( matchListNo )
                        return false;
                    else
                        continue;
                }
                // when there are only layers in the listYes
                // check if these layers are matched, or if the exception works
                if (listNo.size() == 0 ) {
                    if( matchListYes )
                        continue;
                    if (!ruleDefinedListYes && !tiles.contains(t1))
                        continue;
                    return false;
                }

                // there are layers in both lists:
                // no need to consider ruleDefinedListXXX
                if ((matchListYes || !ruleDefinedListYes) && !matchListNo)
                    continue;
                else
                    return false;
            }
        }
    }
    return true;
}

void AutoMapper::applyRule(const QRegion &rule)
{
    const int max_x = mMapWork->width() - rule.boundingRect().left();
    const int max_y = mMapWork->height() - rule.boundingRect().top();

    for (int y = - rule.boundingRect().top(); y <= max_y; y++)
        for (int x = - rule.boundingRect().left(); x <= max_x; x++)
            if (compareLayerTo(mLayerSet, mLayerRuleSets,
                               mLayerRuleNotSets, rule, QPoint(x, y)))
                copyMapRegion(rule, QPoint(x, y), mLayerList);
}

void AutoMapper::autoMap()
{
    foreach (const QRegion &rule, mRules)
        applyRule(rule);
}

void AutoMapper::cleanUpRuleMapLayers()
{
    foreach (const QString &t, mAddedTileLayers) {
        const int layerindex = mMapWork->indexOfLayer(t);
        if (layerindex != -1) {
            TileLayer *t = mMapWork->layerAt(layerindex)->asTileLayer();
            if (t->isEmpty()) {
                mMapDocument->undoStack()->push(
                        new RemoveLayer(mMapDocument, layerindex));
            }
        }
    }


    mLayerList.clear();
    mLayerRuleRegions=0;
    mLayerRuleSets.clear();
    mLayerRuleNotSets.clear();
}

void AutoMapper::cleanUpTilesets()
{
    foreach (Tileset *t, mAddedTilesets) {
        if (mMapWork->isTilesetUsed(t))
            continue;

        const int layerIndex = mMapWork->indexOfTileset(t);
        if (layerIndex != -1) {
            QUndoCommand *cmd = new RemoveTileset(mMapDocument, layerIndex, t);
            mMapDocument->undoStack()->push(cmd);
        }
    }
    // ok remove the entries out of the list,
    // else there would be trouble trying to delete them twice
    mAddedTilesets.clear();
}

AutoMapper::~AutoMapper()
{
    cleanRulesMap();
}

AutomaticMapping::AutomaticMapping(AutoMapper *autoMapper)
{
    mMapDocument = autoMapper->mapDocument();
    Map *map = mMapDocument->map();
    foreach (const QString &layerName, autoMapper->getTouchedLayers()) {
        const int layerindex = map->indexOfLayer(layerName);
        mLayersBefore << map->layerAt(layerindex)->clone();
    }
    autoMapper->autoMap();
    foreach (const QString &layerName, autoMapper->getTouchedLayers()) {
        const int layerindex = map->indexOfLayer(layerName);
        // layerindex exists, because AutoMapper is still alive, dont check
        mLayersAfter << map->layerAt(layerindex)->clone();
    }
}

AutomaticMapping::~AutomaticMapping()
{
    QList<Layer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i)
        delete *i;
    for (i = mLayersBefore.begin(); i != mLayersBefore.end(); ++i)
        delete *i;
}

void AutomaticMapping::undo()
{
    Map *map = mMapDocument->map();
    QList<Layer*>::iterator i;
    for (i = mLayersBefore.begin();i!=mLayersBefore.end(); ++i) {
        const int layerindex = map->indexOfLayer((*i)->name());
        if (layerindex != -1)
            //just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex, (*i)->clone());
    }
}
void AutomaticMapping::redo()
{
    Map *map = mMapDocument->map();
    QList<Layer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i) {
        const int layerindex = (map->indexOfLayer((*i)->name()));
        if (layerindex != -1)
            // just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex, (*i)->clone());
    }
}

Layer *AutomaticMapping::swapLayer(int layerIndex, Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayer();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(layerIndex);
    layerModel->insertLayer(layerIndex, layer);

    if (layerIndex == currentIndex)
        mMapDocument->setCurrentLayer(layerIndex);

    return replaced;
}

AutomaticMappingFileHandler::AutomaticMappingFileHandler(
                            MapDocument *mapDocument, const QString &filePath)
{
    const QString absPath = QFileInfo(filePath).path();
    QFile rulesFile(filePath);

    if (!rulesFile.exists()) {
        QMessageBox::critical(
                    0, tr("AutoMap Error"),
                    tr("No rules file found at:\n%1").arg(filePath));
        return;
    }
    if (!rulesFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(
                    0, tr("AutoMap Error"),
                    tr("Error opening rules file:\n%1").arg(filePath));
        return;
    }

    AutoMapper *autoMapper = new AutoMapper(mapDocument);

    if (autoMapper->setupMapDocumentLayers()) {

        QTextStream in(&rulesFile);
        QString line = in.readLine();

        for (; !line.isNull(); line = in.readLine()) {
            QString rulePath = line.trimmed();
            if (rulePath.isEmpty()
                    || rulePath.startsWith(QLatin1Char('#'))
                    || rulePath.startsWith(QLatin1String("//")))
                continue;

            if (QFileInfo(rulePath).isRelative())
                rulePath = absPath + QLatin1Char('/') + rulePath;

            if (!QFileInfo(rulePath).exists()) {
                QMessageBox::warning(
                            0, tr("AutoMap Warning"),
                            tr("file not found:\n%1").arg(rulePath));
                continue;
            }
            if (rulePath.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive)){
                TmxMapReader mapReader;
                Map *rules = mapReader.read(rulePath);

                if(autoMapper->setupRulesMap(rules)){
                    AutomaticMapping *a = new AutomaticMapping(autoMapper);
                    mapDocument->undoStack()->push(a);
                    autoMapper->cleanRulesMap();
                }
                delete rules;
            }
            if (rulePath.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive)){
                new AutomaticMappingFileHandler(mapDocument, rulePath);
            }
        }
    }
}
