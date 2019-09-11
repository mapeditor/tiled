/*
 * logginginterface.cpp
 * Copyright 2019, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "logginginterface.h"

#include "containerhelpers.h"
#include "layer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "terrain.h"
#include "wangset.h"

namespace Tiled {

unsigned Issue::mNextIssueId = 1;

Issue::Issue()
    : Issue(Error, QString())
{
}

Issue::Issue(Issue::Severity severity,
             const QString &text,
             const std::function<void()> &callback,
             const void *context)
    : mSeverity(severity)
    , mText(text)
    , mCallback(callback)
    , mContext(context)
    , mId(mNextIssueId++)
{
}

void Issue::setCallback(std::function<void ()> callback)
{
    mCallback = std::move(callback);
}

void Issue::addOccurrence(const Issue &issue)
{
    mOccurrences += 1;
    setCallback(issue.callback());
    setContext(issue.context());
}


LoggingInterface::LoggingInterface(QObject *parent)
    : QObject(parent)
{}

LoggingInterface &LoggingInterface::instance()
{
    static LoggingInterface interface;
    return interface;
}

/**
 * Reports an issue by emitting the "issue" signal with the given \a issue.
 *
 * Also emits "warning" or "error" signals as appropriate.
 */
void LoggingInterface::report(const Issue &issue)
{
    switch (issue.severity()) {
    case Issue::Warning:
        emit warning(issue.text());
        break;
    case Issue::Error:
        emit error(issue.text());
        break;
    }

    emit this->issue(issue);
}

/**
 * Logs a \a message of the given \a type.
 *
 * Also reports a matching issue when the type is "WARNING" or "ERROR".
 */
void LoggingInterface::log(OutputType type, const QString &message)
{
    Issue::Severity severity;

    switch (type) {
    default:
    case INFO:
        emit info(message);
        return;
    case WARNING:
        severity = Issue::Warning;
        break;
    case ERROR:
        severity = Issue::Error;
        break;
    }

    // Remove any trailing newlines
    QString messageCopy = message;
    while (messageCopy.endsWith(QLatin1Char('\n')))
        messageCopy.chop(1);

    report(Issue(severity, messageCopy));
}


std::function<void (const OpenFile &)> OpenFile::activated;
std::function<void (const JumpToTile &)> JumpToTile::activated;
std::function<void (const JumpToObject &)> JumpToObject::activated;
std::function<void (const SelectLayer &)> SelectLayer::activated;
std::function<void (const SelectCustomProperty &)> SelectCustomProperty::activated;
std::function<void (const SelectTile &)> SelectTile::activated;


JumpToTile::JumpToTile(const Map *map, QPoint tilePos, const Layer *layer)
    : mapFile(map->fileName)
    , tilePos(tilePos)
    , layerId(layer ? layer->id() : -1)
{
    Q_ASSERT(!mapFile.isEmpty());
}

JumpToObject::JumpToObject(const MapObject *object)
    : mapFile(object->objectGroup()->map()->fileName)
    , objectId(object->id())
{
    Q_ASSERT(!mapFile.isEmpty());
}

SelectLayer::SelectLayer(const Layer *layer)
    : mapFile(layer->map()->fileName)
    , layerId(layer->id())
{
    Q_ASSERT(!mapFile.isEmpty());
}

SelectCustomProperty::SelectCustomProperty(QString fileName,
                                           QString propertyName,
                                           const Object *object)
    : fileName(std::move(fileName))
    , propertyName(std::move(propertyName))
    , objectType(object->typeId())
{
    switch (object->typeId()) {
    case Object::LayerType:
        id = static_cast<const Layer*>(object)->id();
        break;
    case Object::MapObjectType:
        id = static_cast<const MapObject*>(object)->id();
        break;
    case Object::MapType:
        break;
    case Object::ObjectTemplateType:
        break;
    case Object::TerrainType:
        id = static_cast<const Terrain*>(object)->id();
        break;
    case Object::TilesetType:
        break;
    case Object::TileType:
        id = static_cast<const Tile*>(object)->id();
        break;
    case Object::WangSetType: {
        auto wangSet = static_cast<const WangSet*>(object);
        id = indexOf(wangSet->tileset()->wangSets(), wangSet);
        break;
    }
    case Object::WangColorType:
        // not so helpful... would need WangSet index as well
        id = static_cast<const WangColor*>(object)->colorIndex();
        break;
    }
}

SelectTile::SelectTile(const Tile *tile)
    : tileset(tile->tileset()->originalTileset())
    , tilesetFile(tile->tileset()->originalTileset()->fileName())
    , tileId(tile->id())
{}

} // namespace Tiled
