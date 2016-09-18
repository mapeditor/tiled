/*
 * rtbmapsettings.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbmapsettings.h"

#include "map.h"
#include "addremovetileset.h"
#include "addremovelayer.h"
#include "pluginmanager.h"
#include "mapreaderinterface.h"
#include "objectgroup.h"

using namespace Tiled;
using namespace Tiled::Internal;

SharedTileset RTBMapSettings::mTileset = RTBMapSettings::createTileset();
SharedTileset RTBMapSettings::mActionTileset = RTBMapSettings::createActionTileset();
const QColor mapBackgroundColor(0, 0, 0);

RTBMapSettings::RTBMapSettings()
{

}

void RTBMapSettings::loadTileSets(MapDocument *mapDocument)
{
    QUndoStack *undoStack = mapDocument->undoStack();
    undoStack->push(new AddTileset(mapDocument, tileset()));
    undoStack->clear();
}

void RTBMapSettings::setMapSettings(Map *map)
{
    // set map background color
    map->setBackgroundColor(mapBackgroundColor);
}

void RTBMapSettings::createLayers(MapDocument *mapDocument)
{
    Map *map = mapDocument->map();

    QUndoStack *undoStack = mapDocument->undoStack();

    TileLayer *floorLayer = new TileLayer(QCoreApplication::translate("Tiled::Internal::RTBMapSettings", "Floor"), 0, 0, map->width(), map->height());
    undoStack->push(new AddLayer(mapDocument, 0, floorLayer));
    // add object layer
    ObjectGroup *objectLayer = new ObjectGroup(QCoreApplication::translate("Tiled::Internal::RTBMapSettings", "Objects"), 0, 0, map->width(), map->height());
    undoStack->push(new AddLayer(mapDocument, 1, objectLayer));
    // add orb layer
    ObjectGroup *orbObjectLayer = new ObjectGroup(QCoreApplication::translate("Tiled::Internal::RTBMapSettings", "Orbs"), 0, 0, map->width(), map->height());
    orbObjectLayer->setOpacity(0.5);
    undoStack->push(new AddLayer(mapDocument, 2, orbObjectLayer));

    undoStack->clear();
}

void RTBMapSettings::addStarterContent(MapDocument *mapDocument)
{
    // find .json reader
    PluginManager *pm = PluginManager::instance();
    QList<MapReaderInterface*> readers = pm->interfaces<MapReaderInterface>();
    MapReaderInterface *mapReader = 0;
    foreach (MapReaderInterface *reader, readers) {
        foreach (const QString &str, reader->nameFilters()) {
            if (!str.isEmpty() && str.contains(QLatin1String(".json"))) {
                mapReader = reader;
                break;
            }
        }
    }

    // load mapdocument an unify the tilesets
    MapDocument *importMapDocument = MapDocument::load(QLatin1String("://rtb_resources/StarterContent.json"), mapReader);
    mapDocument->unifyTilesets(importMapDocument->map());

    QUndoStack *undoStack = mapDocument->undoStack();

    // set the right size to the layers an add them to the new map
    QSize layerSize = mapDocument->map()->size();
    // floor layer
    TileLayer *layer = importMapDocument->map()->layerAt(RTBMapSettings::FloorID)->clone()->asTileLayer();
    layer->resize(layerSize, QPoint(0, 0));
    undoStack->push(new AddLayer(mapDocument, 0, layer));

    // object layer
    undoStack->push(new AddLayer(mapDocument, 1, importMapDocument->map()->layerAt(RTBMapSettings::ObjectID)->clone()));

    // orb object layer
    undoStack->push(new AddLayer(mapDocument, 2, importMapDocument->map()->layerAt(RTBMapSettings::OrbObjectID)->clone()));

    undoStack->clear();
    importMapDocument->deleteLater();
}
