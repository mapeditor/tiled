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

#include <math.h>

using namespace Tengine;

TenginePlugin::TenginePlugin()
{
}

bool TenginePlugin::write(const Tiled::Map *map, const QString &fileName)
{
    using namespace Tiled;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }
    QTextStream out(&file);

    // Write the header
    QString header = map->property("header");
    foreach (const QString &line, header.split("\\n")) {
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
    // Ability to handle overflow and strings for display
    bool outputLists = false;
    int asciiDisplay = ASCII_MIN;
    int overflowDisplay = 1;
    QHash<QString, Tiled::Properties>::const_iterator i;
    // Add the empty tile
    int numEmptyTiles = 0;
    Properties emptyTile;
    emptyTile["display"] = "?";
    cachedTiles["?"] = emptyTile;
    // Process the map, collecting used display strings as we go
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
                    Tile *tile = tileLayer->cellAt(x, y).tile;
                    if (tile) {
                        currentTile["display"] = tile->property("display");
                        currentTile[layerKey] = tile->property("value");
                    }
                // Process the Object Layer
                } else if (objectLayer) {
                    foreach (const MapObject *obj, objectLayer->objects()) {
                        if (floor(obj->y()) <= y && y <= floor(obj->y() + obj->height())) {
                            if (floor(obj->x()) <= x && x <= floor(obj->x() + obj->width())) {
                                // Check the Object Layer properties if either display or value was missing
                                if (!obj->property("display").isEmpty()) {
                                    currentTile["display"] = obj->property("display");
                                } else if (!objectLayer->property("display").isEmpty()) {
                                    currentTile["display"] = objectLayer->property("display");
                                }
                                if (!obj->property("value").isEmpty()) {
                                    currentTile[layerKey] = obj->property("value");
                                } else if (!objectLayer->property("value").isEmpty()) {
                                    currentTile[layerKey] = objectLayer->property("value");
                                }
                            }
                        }
                    }
                }
            }
            // If the currentTile does not exist in the cache, add it
            if (!cachedTiles.contains(currentTile["display"])) {
                cachedTiles[currentTile["display"]] = currentTile;
            // Otherwise check that it EXACTLY matches the cached one
            // and if not...
            } else if (currentTile != cachedTiles[currentTile["display"]]) {
                // Search the cached tiles for a match
                bool foundInCache = false;
                QString displayString;
                for (i = cachedTiles.constBegin(); i != cachedTiles.constEnd(); ++i) {
                    displayString = i.key();
                    currentTile["display"] = displayString;
                    if (currentTile == i.value()) {
                        foundInCache = true;
                        break;
                    }
                }
                // If we haven't found a match then find a random display string
                // and cache it
                if (!foundInCache) {
                    while (true) {
                        // First try to use the ASCII characters
                        if (asciiDisplay < ASCII_MAX) {
                            displayString = QString(QChar::fromLatin1(asciiDisplay));
                            asciiDisplay++;
                        // Then fall back onto integers
                        } else {
                            displayString = QString::number(overflowDisplay);
                            overflowDisplay++;
                        }
                        currentTile["display"] = displayString;
                        if (!cachedTiles.contains(displayString)) {
                            cachedTiles[displayString] = currentTile;
                            break;
                        } else if (currentTile == cachedTiles[currentTile["display"]]) {
                            break;
                        }
                    }
                }
            }
            // Check the output type
            if (currentTile["display"].length() > 1) {
                outputLists = true;
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
    for (i = cachedTiles.constBegin(); i != cachedTiles.constEnd(); ++i) {
        QString displayString = i.key();
        // Only print the emptyTile definition if there were empty tiles
        if (displayString == QLatin1String("?") && numEmptyTiles == 0) {
            continue;
        }
        // Need to escape " and \ characters
        displayString.replace(QLatin1Char('\\'), "\\\\");
        displayString.replace(QLatin1Char('"'), "\\\"");
        QString args = constructArgs(i.value(), propertyOrder);
        if (!args.isEmpty()) {
            args = QString(", %1").arg(args);
        }
        out << QString("defineTile(\"%1\"%2)").arg(displayString, args) << endl;
    }

    // Check for an ObjectGroup named AddSpot
    out << endl << "-- addSpot section" << endl;
    foreach (Layer *layer, map->layers()) {
        ObjectGroup *objectLayer = layer->asObjectGroup();
        if (objectLayer && objectLayer->name().startsWith("addspot", Qt::CaseInsensitive)) {
            foreach (const MapObject *obj, objectLayer->objects()) {
                QList<QString> propertyOrder;
                propertyOrder.append("type");
                propertyOrder.append("subtype");
                propertyOrder.append("additional");
                QString args = constructArgs(obj->properties(), propertyOrder);
                if (!args.isEmpty()) {
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

    // Check for an ObjectGroup named AddZone
    out << endl << "-- addZone section" << endl;
    foreach (Layer *layer, map->layers()) {
        ObjectGroup *objectLayer = layer->asObjectGroup();
        if (objectLayer && objectLayer->name().startsWith("addzone", Qt::CaseInsensitive)) {
            foreach (MapObject *obj, objectLayer->objects()) {
                QList<QString> propertyOrder;
                propertyOrder.append("type");
                propertyOrder.append("subtype");
                propertyOrder.append("additional");
                QString args = constructArgs(obj->properties(), propertyOrder);
                if (!args.isEmpty()) {
                    args = QString(", %1").arg(args);
                }
                int top_left_x = floor(obj->x());
                int top_left_y = floor(obj->y());
                int bottom_right_x = floor(obj->x() + obj->width());
                int bottom_right_y = floor(obj->y() + obj->height());
                out << QString("addZone({%1, %2, %3, %4}%5)").arg(top_left_x).arg(top_left_y).arg(bottom_right_x).arg(bottom_right_y).arg(args) << endl;
            }
        }
    }

    // Write the map
    QString returnStart;
    QString returnStop;
    QString lineStart;
    QString lineStop;
    QString itemStart;
    QString itemStop;
    QString seperator;
    if (outputLists) {
        returnStart = "{";
        returnStop = "}";
        lineStart = "{";
        lineStop = "},";
        itemStart = "[[";
        itemStop = "]]";
        seperator = ",";
    } else {
        returnStart = "[[";
        returnStop = "]]";
        lineStart = "";
        lineStop = "";
        itemStart = "";
        itemStop = "";
        seperator = "";
    }
    out << endl << "-- ASCII map section" << endl;
    out << "return " << returnStart << endl;
    for (int y = 0; y < height; ++y) {
        out << lineStart;
        for (int x = 0; x < width; ++x) {
            out << itemStart << asciiMap[x + (y * width)] << itemStop << seperator;
        }
        if (y == height - 1) {
            out << lineStop << returnStop;
        } else {
            out << lineStop << endl;
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
        // Special handling of the "additional" property
        if ((propOrder[i] == "additional") && currentValue.isEmpty()) {
            currentValue = constructAdditionalTable(props, propOrder);
        }
        if (!argString.isEmpty()) {
            if (currentValue.isEmpty()) {
                currentValue = "nil";
            }
            argString = QString("%1, %2").arg(currentValue, argString);
        }  else if (!currentValue.isEmpty()) {
            argString = currentValue;
        }
    }
    return argString;
}

// Finds unhandled properties and bundles them into a Lua table
QString TenginePlugin::constructAdditionalTable(Tiled::Properties props, QList<QString> propOrder) const
{
    QString tableString;
    QMap<QString, QString> unhandledProps = QMap<QString, QString>(props);
    // Remove handled properties
    for (int i = 0; i < propOrder.size(); i++) {
        unhandledProps.remove(propOrder[i]);
    }
    // Construct the Lua string
    if (unhandledProps.size() > 0) {
        tableString = "{";
        QMapIterator<QString, QString> i(unhandledProps);
        while (i.hasNext()) {
            i.next();
            tableString = QString("%1%2=%3,").arg(tableString, i.key(), i.value());
        }
        tableString = QString("%1}").arg(tableString);
    }
    return tableString;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Tengine, TenginePlugin)
#endif
