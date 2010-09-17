/*
 * The T-Engine 4 Tiled Plugin
 * Copyright 2010, Mikolai Fajer <mfajer@gmail.com>
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

#include "tengineplugin.h"

#include "math.h"

#include "map.h"
#include "tile.h"
#include "tilelayer.h"
#include "objectgroup.h"
#include "mapobject.h"
#include "properties.h"

#include <QFile>
#include <QTextStream>
#include <QHash>
#include <QList>
#include <QMessageBox>

using namespace Tengine;

TenginePlugin::TenginePlugin()
{
}

bool TenginePlugin::write(const Tiled::Map *map, const QString &fileName)
{
    using namespace Tiled;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        mError = tr("Could not open file for writing.");
        return false;
    }
    QTextStream out(&file);

    // Write the header
    QString header = map->property("header");
    foreach (QString line, header.split("\\n")) {
        out << line << endl;
    }

    const int width = map->width();
    const int height = map->height();

    QList<QString> asciiMap;
    QHash<QString, Tiled::Properties> cachedTiles;
    QList<QString> propertyOrder;
    propertyOrder.append("terrain");
    propertyOrder.append("object");
    propertyOrder.append("actor");
    propertyOrder.append("trap");
    propertyOrder.append("status");
    propertyOrder.append("spot");
    // Add the empty tile
    int numEmptyTiles = 0;
    Properties emptyTile;
    emptyTile["display"] = "?";
    cachedTiles["?"] = emptyTile;
    // Process the map, collecting used display characters as we go
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Properties currentTile = cachedTiles["?"];
            foreach (Layer *layer, map->layers()) {
                // If the layer name does not start with one of the tile properties, skip it
                QString layerKey;
                QListIterator<QString> propertyIterator = propertyOrder;
                while (propertyIterator.hasNext()) {
                    QString currentProperty = propertyIterator.next();
                    if (layer->name().startsWith(currentProperty, Qt::CaseInsensitive)) {
                        layerKey = currentProperty;
                        break;
                    }
                }
                if (layerKey.isEmpty()) {
                    continue;
                }
                TileLayer *tileLayer = layer->asTileLayer();
                ObjectGroup *objectLayer = layer->asObjectGroup();
                // Process the Tile Layer
                if (tileLayer) {
                    Tile *tile = tileLayer->tileAt(x, y);
                    if (tile) {
                        // Have to make sure the display string is only 1 element long
                        currentTile["display"] = tile->property("display")[0];
                        currentTile[layerKey] = tile->property("value");
                    }
                // Process the Object Layer
                } else if (objectLayer) {
                    foreach (MapObject *obj, objectLayer->objects()) {
                        if (floor(obj->y()) <= y and y <= floor(obj->y() + obj->height())) {
                            if (floor(obj->x()) <= x and x <= floor(obj->x() + obj->width())) {
                                // Check the Object Layer properties if either display or value was missing
                                if (not obj->property("display").isEmpty()) {
                                    currentTile["display"] = obj->property("display")[0];
                                } else if (not objectLayer->property("display").isEmpty()) {
                                    currentTile["display"] = objectLayer->property("display")[0];
                                }
                                if (not obj->property("value").isEmpty()) {
                                    currentTile[layerKey] = obj->property("value");
                                } else if (not objectLayer->property("value").isEmpty()) {
                                    currentTile[layerKey] = objectLayer->property("value");
                                }
                            }
                        }
                    }
                }
            }
            // If the currentTile does not exist in the cache, add it
            if (not cachedTiles.contains(currentTile["display"])) {
                cachedTiles[currentTile["display"]] = currentTile;
            // Otherwise check that it EXACTLY matches the cached one
            // And assign a new display character if the currentTile is indeed different
            // ASCII characters between decimals 32 and 126 should be ok
            } else if (currentTile != cachedTiles[currentTile["display"]]) {
                for (int i = ASCII_MIN; x <= ASCII_MAX; ++i) {
                    QString selectedChar = QString(QChar::fromAscii(i));
                    currentTile["display"] = selectedChar;
                    if (cachedTiles.contains(selectedChar)) {
                        if (currentTile == cachedTiles[selectedChar]) {
                            break;
                        }
                    } else {
                        cachedTiles[currentTile["display"]] = currentTile;
                        break;
                    }
                    // We ran out of possibilities!
                    if (i == ASCII_MAX) {
                        mError = "Ran out of unique ASCII characters.";
                        return false;
                    }
                }
            }
            // Check if we are still the emptyTile
            if (currentTile == emptyTile) {
                numEmptyTiles++;
            }
            // Finally add the character to the asciiMap
            asciiMap.append(currentTile["display"]);
        }
    }
    // Write the definitions to the file
    out << "-- defineTile section" << endl;
    QHash<QString, Tiled::Properties>::const_iterator i;
    for (i = cachedTiles.constBegin(); i != cachedTiles.constEnd(); i++) {
        QString displayChar = i.key();
        // Handle the special characters
        if (displayChar == "\"") {
            displayChar = "\\\"";
        } else if (displayChar == "\\") {
            displayChar = "\\\\";
        } else if (displayChar == "\'") {
            displayChar = "\\\'";
        // Only print the emptyTile definition if there were empty tiles
        } else if (displayChar == "?" and numEmptyTiles == 0) {
            continue;
        }
        QString args = constructArgs(i.value(), propertyOrder);
        if (not args.isEmpty()) {
            args = QString(", %1").arg(args);
        }
        out << QString("defineTile(\"%1\"%2)").arg(displayChar, args) << endl;
    }

    // Check for an ObjectGroup named AddSpot
    out << endl << "-- addSpot section" << endl;
    foreach (Layer *layer, map->layers()) {
        ObjectGroup *objectLayer = layer->asObjectGroup();
        if (objectLayer and objectLayer->name().toLower() == "addspot") {
            foreach (MapObject *obj, objectLayer->objects()) {
                QList<QString> propertyOrder;
                propertyOrder.append("type");
                propertyOrder.append("subtype");
                QString args = constructArgs(obj->properties(), propertyOrder);
                if (not args.isEmpty()) {
                    args = QString(", %1").arg(args);
                }
                for (int y = floor(obj->y()); y <= floor(obj->y() + obj->height()); ++y) {
                    for (int x = floor(obj->x()); x <= floor(obj->x() + obj->width()); ++x) {
                        out << QString("addSpot({%1, %2}%3)").arg(x).arg(y).arg(args) << endl;
                    }
                }
            }
        }
    }
    // Write the map
    out << endl << "-- ASCII map section" << endl;
    out << "return [[" << endl;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            out << asciiMap[x + (y * width)];
        }
        if (y == height - 1){
            out << "]]";
        } else {
            out << endl;
        }
    }

    // And close the file
    file.close();

    return true;
}

QString TenginePlugin::nameFilter() const
{
    return tr("T-Engine4 map files (*.lua)");
}

QString TenginePlugin::errorString() const
{
    return mError;
}

QString TenginePlugin::constructArgs(Tiled::Properties props, QList<QString> propOrder) const
{
    QString argString;
    // We work backwards so we don't have to include a bunch of nils
    for (int i = propOrder.size() - 1; i >= 0; --i) {
        QString currentValue = props[propOrder[i]];
        if (not argString.isEmpty()) {
            if (currentValue.isEmpty()) {
                currentValue = "nil";
            }
            argString = QString("%1, %2").arg(currentValue, argString);
        }  else if (not currentValue.isEmpty()) {
            argString = currentValue;
        }
    }
    return argString;
}

Q_EXPORT_PLUGIN2(Tengine, TenginePlugin)
