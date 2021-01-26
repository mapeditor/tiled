/*
 * YY Tiled Plugin
 * Copyright 2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "yyplugin.h"

#include "jsonwriter.h"
#include "logginginterface.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include "qtcompat_p.h"

using namespace Tiled;
using namespace Yy;

template <typename T>
static T optionalProperty(const Object *object, const QString &name, const T &def)
{
    const QVariant var = object->resolvedProperty(name);
    return var.isValid() ? var.value<T>() : def;
}

template <typename T>
static void writeProperty(JsonWriter &writer,
                          const Object *object,
                          const QString &propertyName,
                          const char *memberName,
                          const T &def)
{
    const T value = optionalProperty(object, propertyName, def);
    writer.writeMember(memberName, value);
}

template <typename T>
static void writeProperty(JsonWriter &writer,
                          const Object *object,
                          const char *name,
                          const T &def)
{
    writeProperty(writer, object, QString::fromLatin1(name), name, def);
}

static bool checkIfViewsDefined(const Map *map)
{
    LayerIterator iterator(map);
    while (const Layer *layer = iterator.next()) {

        if (layer->layerType() != Layer::ObjectGroupType)
            continue;

        const ObjectGroup *objectLayer = static_cast<const ObjectGroup*>(layer);

        for (const MapObject *object : objectLayer->objects()) {
            const QString type = object->effectiveType();
            if (type == "view")
                return true;
        }
    }

    return false;
}

YyPlugin::YyPlugin()
{
}

bool YyPlugin::write(const Map *map, const QString &fileName, Options options)
{
    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    JsonWriter json(file.device());

    json.setMinimize(options.testFlag(WriteMinimized));

    json.writeStartObject();

    writeProperty(json, map, "isDnd", false);
    writeProperty(json, map, "volume", 1.0);
    json.writeMember("parentRoom", QJsonValue(QJsonValue::Null));    // TODO: Provide a way to set this?

    // Check if views are defined
    const bool enableViews = checkIfViewsDefined(map);

    // Write out views
    // Last view in Object layer is the first view in the room
    if (enableViews) {
        json.writeStartArray("views");
        int viewCount = 0;
        for (const Layer *layer : map->objectGroups()) {
            const ObjectGroup *objectLayer = static_cast<const ObjectGroup*>(layer);

            for (const MapObject *object : objectLayer->objects()) {
                const QString type = object->effectiveType();
                if (type != "view")
                    continue;

                // GM only has 8 views so drop anything more than that
                if (viewCount > 7) {
                    Tiled::ERROR(QLatin1String("YY plugin: Can't export more than 8 views."),
                                 Tiled::JumpToObject { object });
                    break;
                }

                viewCount++;
                json.writeStartObject();
                json.setMinimize(true);

                writeProperty(json, object, "inherit", false);
                json.writeMember("visible", object->isVisible());
                // Note: GM only supports ints for positioning
                // so views could be off if user doesn't align to whole number
                json.writeMember("xview", QString::number(qRound(object->x())));
                json.writeMember("yview", QString::number(qRound(object->y())));
                json.writeMember("wview", QString::number(qRound(object->width())));
                json.writeMember("hview", QString::number(qRound(object->height())));
                // Round these incase user adds properties as floats and not ints
                json.writeMember("xport", QString::number(qRound(optionalProperty(object, "xport", 0.0))));
                json.writeMember("yport", QString::number(qRound(optionalProperty(object, "yport", 0.0))));
                json.writeMember("wport", QString::number(qRound(optionalProperty(object, "wport", 1024.0))));
                json.writeMember("hport", QString::number(qRound(optionalProperty(object, "hport", 768.0))));
                json.writeMember("hborder", QString::number(qRound(optionalProperty(object, "hborder", 32.0))));
                json.writeMember("vborder", QString::number(qRound(optionalProperty(object, "vborder", 32.0))));
                json.writeMember("hspeed", QString::number(qRound(optionalProperty(object, "hspeed", -1.0))));
                json.writeMember("vspeed", QString::number(qRound(optionalProperty(object, "vspeed", -1.0))));
                json.writeMember("objectId", QJsonValue::Null);    // TODO: Provide a way to set this?

                json.setMinimize(false);
                json.writeEndObject();
            }
        }

        json.writeEndArray();
    }

    json.writeStartArray("layers");

    LayerIterator iterator(map);
    while (const Layer *layer = iterator.next()) {
        // TODO
        Q_UNUSED(layer);
    }

    json.writeEndArray();

    writeProperty(json, map, "inheritLayers", false);
    writeProperty(json, map, "creationCodeFile", QString());
    writeProperty(json, map, "inheritCode", false);

    json.writeStartArray("instanceCreationOrder");
    // TODO
    //      {"name":"inst_967BF0","path":"rooms/Room1/Room1.yy",},
    //      {"name":"inst_1D2061C1","path":"rooms/Room1/Room1.yy",},
    //      {"name":"inst_6C3C7802","path":"rooms/Room1/Room1.yy",},
    json.writeEndArray();

    writeProperty(json, map, "inheritCreationOrder", false);
    json.writeMember("sequenceId", QJsonValue::Null);

    const int mapPixelWidth = map->tileWidth() * map->width();
    const int mapPixelHeight = map->tileHeight() * map->height();

    json.writeStartObject("roomSettings");
    writeProperty(json, map, "inheritRoomSettings", false);
    json.writeMember("Width", mapPixelWidth);
    json.writeMember("Height", mapPixelHeight);
    writeProperty(json, map, "persistent", false);
    json.writeEndObject();

    json.writeStartObject("viewSettings");
    writeProperty(json, map, "inheritViewSettings", false);
    writeProperty(json, map, "enableViews", enableViews);
    writeProperty(json, map, "clearViewBackground", false);
    writeProperty(json, map, "clearDisplayBuffer", true);
    json.writeEndObject();

    json.writeStartObject("physicsSettings");
    writeProperty(json, map, "inheritPhysicsSettings", false);
    writeProperty(json, map, "PhysicsWorld", false);
    writeProperty(json, map, "PhysicsWorldGravityX", 0.0);
    writeProperty(json, map, "PhysicsWorldGravityY", 10.0);
    writeProperty(json, map, "PhysicsWorldPixToMetres", 0.1);
    json.writeEndObject();

    json.writeStartObject("parent");
    const QString path = optionalProperty(map, "path", QLatin1String("folders/Rooms.yy"));
    json.writeMember("name", QFileInfo(path).completeBaseName());
    json.writeMember("path", path);
    json.writeEndObject();

    writeProperty(json, map, "resourceVersion", QString("1.0"));
    writeProperty(json, map, "name", QFileInfo(fileName).completeBaseName());
    const QString tags = optionalProperty(map, "tags", QString());
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    const QStringList tagList = tags.split(QLatin1Char(','), QString::SkipEmptyParts);
#else
    const QStringList tagList = tags.split(QLatin1Char(','), Qt::SkipEmptyParts);
#endif
    json.writeMember("tags", QJsonArray::fromStringList(tagList));
    json.writeMember("resourceType", "GMRoom");

    json.writeEndObject();
    json.writeEndDocument();

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString YyPlugin::errorString() const
{
    return mError;
}

QString YyPlugin::nameFilter() const
{
    return tr("GameMaker Studio 2 Files (*.yy)");
}

QString YyPlugin::shortName() const
{
    return QStringLiteral("yy");
}
