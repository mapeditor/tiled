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
#include "tilesetmanager.h"
#include "tmxmapreader.h"

#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>

using namespace Tiled;
using namespace Tiled::Internal;

AutoMapper::AutoMapper(MapDocument *workingDocument)
    : mMapDocument(workingDocument)
    , mMapWork(workingDocument ? workingDocument->map() : 0)
    , mMapRules(0)
    , mLayerRuleRegions(0)
    , mLayerSet(0)
{

    connect(mMapDocument, SIGNAL(layerAdded(int)), SLOT(layerAdd(int)));
    connect(mMapDocument->layerModel(), SIGNAL(layerAboutToBeRemoved(int)),
            SLOT(layerRemove(int)));
    connect(mMapDocument->layerModel(), SIGNAL(layerAboutToBeRenamed(int)),
            SLOT(layerRemove(int)));
    connect(mMapDocument->layerModel(), SIGNAL(layerRenamed(int)),
            SLOT(layerAdd(int)));
}

AutoMapper::~AutoMapper()
{
    cleanUpRulesMap();
}

QSet<QString> AutoMapper::getTouchedLayers() const
{
    return mTouchedLayers;
}

bool AutoMapper::prepareLoad(Map *rules, const QString &rulePath)
{
    if (!setupMapDocumentLayers())
        return false;

    if (!setupRulesMap(rules, rulePath))
        return false;

    if (!setupRuleMapLayers())
        return false;

    if (!setupRulesUsedCheck())
        return false;

    if (!setupRuleList())
        return false;

    return true;
}

bool AutoMapper::setupMapDocumentLayers()
{
    Q_ASSERT(!mLayerSet);
    mLayerSet = findTileLayer(mMapWork, QLatin1String("set"));

    if (!mLayerSet)
        return false;

    return true;
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

bool AutoMapper::setupRulesMap(Map *rules, const QString &rulePath)
{
    Q_ASSERT(!mMapRules);

    mMapRules = rules;
    mRulePath = rulePath;

    QVariant p = rules->property(QLatin1String("DeleteTiles"));

    // defaulting to false, if there is no such property
    mDeleteTiles = p.toBool();

    QVariant q = rules->property(QLatin1String("AutomappingRadius"));
    mAutoMappingRadius = q.toInt();

    return true;
}

bool AutoMapper::setupRuleMapLayers()
{
    Q_ASSERT(mLayerList.isEmpty());
    Q_ASSERT(!mLayerRuleRegions);
    Q_ASSERT(mLayerRuleSets.isEmpty());
    Q_ASSERT(mLayerRuleNotSets.isEmpty());
    Q_ASSERT(mAddedTilesets.isEmpty());

    QString prefix = QLatin1String("rule");

    foreach (Layer *layer, mMapRules->layers()) {
    if (TileLayer *tileLayer = layer->asTileLayer()) {

        if (!tileLayer->name().startsWith(
                    QLatin1String("rule"), Qt::CaseInsensitive))
            continue;

        // strip leading prefix, to make handling better
        QString layername = tileLayer->name();
        layername.remove(0, prefix.length());

        if (layername.startsWith(
                QLatin1String("set"), Qt::CaseInsensitive)) {
            mLayerRuleSets.append(tileLayer);
            continue;
        }

        if (layername.startsWith(
                QLatin1String("notset"), Qt::CaseInsensitive)) {
            mLayerRuleNotSets.append(tileLayer);
            continue;
        }

        if (layername.startsWith(
                QLatin1String("regions"), Qt::CaseInsensitive)) {
            mLayerRuleRegions = tileLayer;
            continue;
        }

        int pos = layername.indexOf(QLatin1Char('_'), Qt::CaseInsensitive) + 1;
        QString group = layername.left(pos) ;

        QString name = layername.right(layername.size() - pos);

        mTouchedLayers |= name;

        TileLayer *t = findTileLayer(mMapWork, name);
        // if there is no such layer, setup later
        if (!t) {
            mAddLayers.append(name);
//            const int index = mMapWork->layerCount();
//            t = new TileLayer(name, 0, 0,
//                              mMapWork->width(), mMapWork->height());
//            mMapDocument->undoStack()->push(
//                        new AddLayer(mMapDocument, index, t));
//            mMapDocument->setCurrentLayer(index);
//            mAddedTileLayers.append(t->name());
        }

        QPair<TileLayer*, TileLayer*> addPair(tileLayer, t);

        QList<QPair<TileLayer*, TileLayer*> > *list = 0;
        int j = 0;

        // put the list at the right location of mLayerList (a list of lists)
        while ( !list && j != mLayerList.size() ) {
            if (mLayerList.at(j)->at(0).first->name().startsWith(prefix + group))
                list = mLayerList.at(j);
            j++;
        }

        // now add the addPair data, which contains the current tilelayer
        if (!list) {
            list = new QList<QPair<TileLayer*, TileLayer*> >();
            list->append(addPair);
            mLayerList.append(list);
        } else {
            list->append(addPair);
        }
    } // if (asTileLayer)
    } // foreach (layer)

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
        error = mRulePath + QLatin1Char('\n') + error;
        QMessageBox msgBox;
        msgBox.setText(error);
        msgBox.exec();
        return false;
    }

    return true;
}

bool AutoMapper::setupRulesUsedCheck()
{
    QList<Tileset*> tilesetWork = mLayerSet->usedTilesets().toList();
    foreach (TileLayer *tl, mLayerRuleSets)
        foreach (Tileset *ts, tl->usedTilesets())
            if (ts->findSimilarTileset(tilesetWork))
                return true;

    foreach (TileLayer *tl, mLayerRuleNotSets)
        foreach (Tileset *ts, tl->usedTilesets())
            if (ts->findSimilarTileset(tilesetWork))
                return true;

    return false;
}

bool AutoMapper::setupRuleList()
{
    Q_ASSERT(mRules.isEmpty());
    Q_ASSERT(mLayerRuleRegions);

    for (int y = 1; y < mMapRules->height(); y++ ) {
        for (int x = 1; x < mMapRules->width();  x++ ) {
            if (mLayerRuleRegions->tileAt(x, y)
                    && !isPartOfExistingRule(QPoint(x, y))) {
                QRegion rule = createRule(x, y);
                mRules << rule;
            }
        }
    }
    return true;
}

bool AutoMapper::isPartOfExistingRule(const QPoint &p) const
{
    foreach (const QRegion &region, mRules)
        if (region.contains(p))
            return true;

    return false;
}

QRegion AutoMapper::createRule(int x, int y) const
{
    Q_ASSERT(mLayerRuleRegions);
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

bool AutoMapper::prepareAutoMap()
{
    if (!setupMissingLayers())
        return false;

    if (!setupTilesets(mMapRules, mMapWork))
        return false;

    return true;
}

bool AutoMapper::setupMissingLayers()
{
    foreach (QString name, mAddLayers) {
        const int index = mMapWork->layerCount();

        TileLayer *t = new TileLayer(name, 0, 0,
                          mMapWork->width(), mMapWork->height());
        mMapDocument->undoStack()->push(
                    new AddLayer(mMapDocument, index, t));

        mAddedTileLayers.append(name);
    }
    return true;
}

void AutoMapper::layerAdd(int index)
{
    TileLayer *layer = mMapWork->layerAt(index)->asTileLayer();

    if (!layer)
        return;

    QString name = layer->name();
    if (mAddLayers.contains(name, Qt::CaseInsensitive)) {
        mAddLayers.removeAt(mAddLayers.indexOf(name));

        QList<QList<QPair<TileLayer*, TileLayer*> >* >::const_iterator j;
        QList<QPair<TileLayer*, TileLayer*> >::iterator i;

        // update layer translation table
        for (j = mLayerList.constBegin(); j != mLayerList.constEnd(); ++j)
            for (i = (*j)->begin(); i != (*j)->end(); ++i)
                if (i->first->name().endsWith(name, Qt::CaseInsensitive)) {
                    QPair<TileLayer*, TileLayer*> updatePair(i->first, layer);
                    *i = updatePair;
                }
    }
}

void AutoMapper::layerRemove(int index)
{
    TileLayer *layer = mMapWork->layerAt(index)->asTileLayer();

    if (!layer)
        return;

    QString name = layer->name();
    if (mTouchedLayers.contains(name)) {
        mAddLayers.append(name);

        QList<QList<QPair<TileLayer*, TileLayer*> >* >::const_iterator j;
        QList<QPair<TileLayer*, TileLayer*> >::iterator i;

        // remove all pointers to that layer
        for (j = mLayerList.constBegin(); j != mLayerList.constEnd(); ++j)
            for (i = (*j)->begin(); i != (*j)->end(); ++i)
                if (i->second && (i->second->name() == name)) {
                    QPair<TileLayer*, TileLayer*> updatePair(i->first, 0);
                    *i = updatePair;
                }
    }

    // maybe there are more layers with this name, so check if there is another
    // layer to take that place
    for (int i = 0; i < mMapWork->layerCount(); i++) {
        Layer *l = mMapWork->layerAt(i);
        // Do not take that layer, which was removed.
        if (layer !=l && l->name() == name) {
            layerAdd(i);
            break;
        }
    }
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

        Tileset *replacement = tileset->findSimilarTileset(existingTilesets);
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

        TilesetManager *tilesetManager = TilesetManager::instance();
        tilesetManager->addReference(replacement);
        tilesetManager->removeReference(tileset);
    }
    return true;
}

void AutoMapper::autoMap(QRegion *where)
{
    // first resize the active area
    if (mAutoMappingRadius) {
        QRegion *n = new QRegion();
        foreach (QRect r, where->rects()) {
            *n += r.adjusted(- mAutoMappingRadius,
                             - mAutoMappingRadius,
                             + mAutoMappingRadius,
                             + mAutoMappingRadius);
        }
        *where += *n;
    }

    // delete all the relevant area, if the property "DeleteTiles" is set
    if (mDeleteTiles) {
        QList<QList<QPair<TileLayer*, TileLayer*> >* >::const_iterator j;
        QList<QPair<TileLayer*, TileLayer*> >::const_iterator i;
        for (j = mLayerList.constBegin(); j != mLayerList.constEnd(); ++j)
            for (i = (*j)->constBegin(); i != (*j)->constEnd(); ++i)
                clearRegion(i->second, *where);
    }

    // Increase the given region where the next automapper should work.
    // This needs to be done, so you can rely on the order of the rules at all
    // locations
    QRegion ret;
    foreach (const QRect &rect, where->rects())
        foreach (const QRegion &rule, mRules) {
            // at the moment the parallel execution does not work yet
            // TODO: make multithreading available!
            // either by dividing the rules or the region to multiple threads
            ret = ret.united(applyRule(rule, rect));
        }
    *where = where->united(ret);
}

void AutoMapper::clearRegion(TileLayer *dstLayer, const QRegion &where)
{
    QRegion region = where.intersected(dstLayer->bounds());
    foreach (QRect r, region.rects())
        for (int x = r.left(); x <= r.right(); x++)
            for (int y = r.top(); y <= r.bottom(); y++)
                dstLayer->setTile(x, y, 0);
}

static bool compareLayerTo(TileLayer *l1, QVector<TileLayer*> listYes,
            QVector<TileLayer*> listNo, const QRegion &r1, QPoint offset);

QRect AutoMapper::applyRule(const QRegion &rule, const QRect &where)
{
    QRect ret;

    QRect rbr = rule.boundingRect();

    // Since the rule itself is translated, we need to adjust the borders of the
    // loops. Decrease the size at all sides by one: There must be at least one
    // tile overlap to the rule.
    const int min_x = where.left() - rbr.left() - rbr.width() + 1 ;
    const int min_y = where.top() - rbr.top() - rbr.height() + 1;

    const int max_x = where.right() - rbr.left() + rbr.height() - 1;
    const int max_y = where.bottom() - rbr.top() + rbr.width() - 1;

    for (int y = min_y; y <= max_y; y++)
        for (int x = min_x; x <= max_x; x++)
            if (compareLayerTo(mLayerSet, mLayerRuleSets,
                               mLayerRuleNotSets, rule, QPoint(x, y))) {
                int r = 0;
                // choose by chance which group of rule_layers should be used:
                if (mLayerList.size()>1)
                    r = qrand() % mLayerList.size();
                copyMapRegion(rule, QPoint(x, y), *mLayerList.at(r));
                ret = ret.united(rbr.translated(QPoint(x, y)));
            }

    return ret;
}

/**
 * returns a list of all tiles which can be found within all tile layers
 * within the given region.
 */
static QVector<Tile*> tilesInRegion(QVector<TileLayer*> list, const QRegion &r)
{
    QVector<Tile*> tiles;
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
 * This comparision will determine if a rule of automapping matches,
 * so if this rule is applied at this region given
 * by a QRegion and Offset given by a QPoint.
 *
 * This compares the tile layer l1 ("set" layer) to several others given
 * in the QList listYes (ruleSet) and OList listNo (ruleNotSet).
 * The tile layer l1 is examined at QRegion r1 + offset
 * The tile layers within listYes and listNo are examined at QRegion r1.
 *
 * Basically all matches between l1 and a layer of listYes are considered
 * good, while all matches between l1 and listNo are considered bad and lead to
 * canceling the comparison, returning false.
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
 *          to all available tiles in listYes, there must be at least
 *          one layer, in which there is a match of tiles of l1 and listYes
 *          to consider this position good.
 *          In listNo there must not be a match to consider this position
 *          good.
 *          If there are no tiles within all available tiles within all layers
 *          of one list, all tiles in l1 are considered good,
 *          while inspecting this list.
 *          All available tiles are all tiles within the whole rule region in
 *          all tile layers of the list.
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
static bool compareLayerTo(TileLayer *l1, QVector<TileLayer*> listYes,
            QVector<TileLayer*> listNo, const QRegion &r1, QPoint offset)
{
    if (listYes.size() == 0 && listNo.size() == 0)
        return false;

    QVector<Tile*> tiles;
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
                if (listYes.size() == 0 ) {
                    if (matchListNo)
                        return false;
                    else
                        continue;
                }
                // when there are only layers in the listYes
                // check if these layers are matched, or if the exception works
                if (listNo.size() == 0 ) {
                    if (matchListYes)
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

void AutoMapper::copyMapRegion(const QRegion &region, QPoint offset,
                               const QList< QPair<TileLayer*, TileLayer*> > &layerTranslation)
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

void AutoMapper::copyRegion(TileLayer *srcLayer, int srcX, int srcY,
                            int width, int height,
                            TileLayer *dstLayer, int dstX, int dstY)
{
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            if (Tile *t = srcLayer->tileAt(srcX + x, srcY + y))
                // this is without graphics update, it's done afterwards for all
                dstLayer->setTile(dstX + x, dstY + y, t);
}

void AutoMapper::cleanAll()
{
    cleanTilesets();
}

void AutoMapper::cleanTilesets()
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
    mAddedTilesets.clear();
}

void AutoMapper::cleanUpRulesMap()
{
    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->removeReferences(mMapRules->tilesets());

    delete mMapRules;
    mMapRules = 0;

    cleanUpRuleMapLayers();
    cleanTilesets();
    mRules.clear();
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

    QList<QList<QPair<TileLayer*, TileLayer*> >* >::const_iterator j;
    for (j = mLayerList.constBegin(); j != mLayerList.constEnd(); ++j)
        delete (*j);

    mLayerList.clear();
    // do not delete mLayerRuleRegions, it is owned by the map mapWork, which
    // cares for it
    mLayerRuleRegions = 0;
    mLayerRuleSets.clear();
    mLayerRuleNotSets.clear();
}

AutoMapperWrapper::AutoMapperWrapper(MapDocument *mapDocument, QVector<AutoMapper*> autoMapper, QRegion *where)
{
    mMapDocument = mapDocument;
    Map *map = mMapDocument->map();

    QSet<QString> touchedlayers;
    foreach (AutoMapper *a, autoMapper) {
        a->prepareAutoMap();
        touchedlayers|= a->getTouchedLayers();
    }
    foreach (const QString &layerName, touchedlayers) {
        const int layerindex = map->indexOfLayer(layerName);
        mLayersBefore << map->layerAt(layerindex)->clone();
    }

    foreach (AutoMapper *a, autoMapper) {
        a->autoMap(where);
    }

    foreach (const QString &layerName, touchedlayers) {
        const int layerindex = map->indexOfLayer(layerName);
        // layerindex exists, because AutoMapper is still alive, dont check
        mLayersAfter << map->layerAt(layerindex)->clone();
    }
    foreach (AutoMapper *a, autoMapper) {
        a->cleanAll();
    }
}

AutoMapperWrapper::~AutoMapperWrapper()
{
    QVector<Layer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i)
        delete *i;
    for (i = mLayersBefore.begin(); i != mLayersBefore.end(); ++i)
        delete *i;
}

void AutoMapperWrapper::undo()
{
    Map *map = mMapDocument->map();
    QVector<Layer*>::iterator i;
    for (i = mLayersBefore.begin(); i != mLayersBefore.end(); ++i) {
        const int layerindex = map->indexOfLayer((*i)->name());
        if (layerindex != -1)
            //just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex, (*i)->clone());
    }
}
void AutoMapperWrapper::redo()
{
    Map *map = mMapDocument->map();
    QVector<Layer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i) {
        const int layerindex = (map->indexOfLayer((*i)->name()));
        if (layerindex != -1)
            // just put a clone, so the saved layers wont be altered by others.
            delete swapLayer(layerindex, (*i)->clone());
    }
}

Layer *AutoMapperWrapper::swapLayer(int layerIndex, Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayer();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(layerIndex);
    layerModel->insertLayer(layerIndex, layer);

    if (layerIndex == currentIndex)
        mMapDocument->setCurrentLayer(layerIndex);

    return replaced;
}

AutomaticMappingManager *AutomaticMappingManager::mInstance = 0;

AutomaticMappingManager::AutomaticMappingManager(QObject *parent)
    : QObject(parent)
    , mMapDocument(0)
    , mLoaded(false)
    , mWatcher(new QFileSystemWatcher(this))
{
    connect(mWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(fileChanged(QString)));
    mChangedFilesTimer.setInterval(100);
    mChangedFilesTimer.setSingleShot(true);
    connect(&mChangedFilesTimer, SIGNAL(timeout()),
            this, SLOT(fileChangedTimeout()));
}

AutomaticMappingManager::~AutomaticMappingManager()
{
    cleanUp();
    delete mWatcher;
}

AutomaticMappingManager *AutomaticMappingManager::instance()
{
    if (!mInstance)
        mInstance = new AutomaticMappingManager(0);

    return mInstance;
}

void AutomaticMappingManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

void AutomaticMappingManager::automap()
{
    if (!mMapDocument)
        return;

    int w = mMapDocument->map()->width();
    int h = mMapDocument->map()->height();
    TileLayer *l = new TileLayer(QLatin1String("set"), 0 , 0, 0, 0);
    automap(QRect(0, 0, w, h), l, true);
    delete l;
}

void AutomaticMappingManager::automap(QRegion where, Layer *l, bool verbose)
{
    if (!mMapDocument)
        return;

    if (l->name() != QLatin1String("set"))
        return;

    if (!mLoaded) {
        const QString mapPath = QFileInfo(mMapDocument->fileName()).path();
        const QString rulesFileName = mapPath + QLatin1String("/rules.txt");
        if (loadFile(rulesFileName, verbose))
            mLoaded = true;
    }

    Map *map = mMapDocument->map();

    QString layer = map->layerAt(mMapDocument->currentLayer())->name();

    // use a pointer to the region, so each automapper can manipulate it and the
    // following automappers do see the impact
    QRegion *passedRegion = new QRegion(where);



    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Apply AutoMap rules"));
    AutoMapperWrapper *aw = new AutoMapperWrapper(mMapDocument, mAutoMappers, passedRegion);
    undoStack->push(aw);
    undoStack->endMacro();

    mMapDocument->emitRegionChanged(*passedRegion);
    delete passedRegion;
    mMapDocument->setCurrentLayer(map->indexOfLayer(layer));
}

bool AutomaticMappingManager::loadFile(const QString &filePath, bool verbose)
{
    bool ret = true;
    const QString absPath = QFileInfo(filePath).path();
    QFile rulesFile(filePath);

    if (!rulesFile.exists()) {
        if (verbose)
            QMessageBox::critical(
                    0, tr("AutoMap Error"),
                    tr("No rules file found at:\n%1").arg(filePath));
        return false;
    }
    if (!rulesFile.open(QIODevice::ReadOnly)) {
        if (verbose)
            QMessageBox::critical(
                    0, tr("AutoMap Error"),
                    tr("Error opening rules file:\n%1").arg(filePath));
        return false;
    }

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
            if (verbose)
                QMessageBox::warning(0, tr("AutoMap Warning"),
                        tr("file not found:\n%1").arg(rulePath));
            ret = false;
            continue;
        }
        if (rulePath.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive)){
            TmxMapReader mapReader;

            Map *rules = mapReader.read(rulePath);

            TilesetManager *tilesetManager = TilesetManager::instance();
            tilesetManager->addReferences(rules->tilesets());

            if (!rules) {
                if (verbose)
                        QMessageBox::warning(
                                0, tr("Error Opening Rules Map"),
                                mapReader.errorString());
                ret = false;
                continue;
            }
            AutoMapper *autoMapper;
            autoMapper = new AutoMapper(mMapDocument);

            if (autoMapper->prepareLoad(rules, rulePath))
                mAutoMappers.append(autoMapper);
            else
                delete autoMapper;

        }
        if (rulePath.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive)){
            if (!loadFile(rulePath, verbose))
                ret = false;
        }
    }
    return ret;
}

void AutomaticMappingManager::setMapDocument(MapDocument *mapDocument)
{
    cleanUp();
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument)
        connect(mMapDocument, SIGNAL(regionEdited(QRegion,Layer*)),
                this, SLOT(automap(QRegion,Layer*)));
    mLoaded = false;

}

void AutomaticMappingManager::cleanUp()
{
    foreach (AutoMapper *autoMapper, mAutoMappers) {
        delete autoMapper;
    }
    mAutoMappers.clear();
}

void AutomaticMappingManager::fileChanged(const QString &path)
{
    /*
     * Use a one-shot timer and wait a little, to be sure there are no
     * further modifications within very short time.
     */
    if (!mChangedFiles.contains(path)) {
        mChangedFiles.insert(path);
        mChangedFilesTimer.start();
    }
}

void AutomaticMappingManager::fileChangedTimeout()
{
    for (int i = 0; i != mAutoMappers.size(); i++) {
        AutoMapper *am = mAutoMappers.at(i);
        QString fileName = am->ruleSetPath();
        if (mChangedFiles.contains(fileName)) {
            delete am;

            TmxMapReader mapReader;
            Map *rules = mapReader.read(fileName);
            if (!rules) {
                mAutoMappers.remove(i);
                continue;
            }

            AutoMapper *autoMapper = new AutoMapper(mMapDocument);
            if (autoMapper->prepareLoad(rules, fileName)) {
                mAutoMappers.replace(i, autoMapper);
            } else {
                delete autoMapper;
                mAutoMappers.remove(i);
            }
        }
    }
    mChangedFiles.clear();
}
