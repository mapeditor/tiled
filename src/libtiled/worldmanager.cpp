/*
 * worldmanager.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "worldmanager.h"

#include "logginginterface.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include <QDebug>

#include "qtcompat_p.h"

namespace Tiled {

WorldManager *WorldManager::mInstance;

WorldManager::WorldManager()
{
    connect(&mFileSystemWatcher, &FileSystemWatcher::filesChanged,
            this, &WorldManager::reloadWorldFiles);
}

WorldManager::~WorldManager()
{
    qDeleteAll(mWorlds);
}

WorldManager &WorldManager::instance()
{
    if (!mInstance)
        mInstance = new WorldManager;

    return *mInstance;
}

void WorldManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

void WorldManager::reloadWorldFiles(const QStringList &fileNames)
{
    bool changed = false;

    for (const QString &fileName : fileNames) {

        if (mWorlds.contains(fileName)) {
            auto world = privateLoadWorld(fileName);
            if (world) {
                std::unique_ptr<World> oldWorld { mWorlds.take(fileName) };
                oldWorld->clearErrorsAndWarnings();

                mWorlds.insert(fileName, world.release());

                changed = true;
            }
        }
    }

    if (changed)
        emit worldsChanged();
}

static QString jsonValueToString(const QJsonValue &value)
{
    switch (value.type()) {
    case QJsonValue::Null:
        return QLatin1String("null");
    case QJsonValue::Bool:
        return value.toBool() ? QLatin1String("true") : QLatin1String("false");
    case QJsonValue::Double:
        return QString::number(value.toDouble());
    case QJsonValue::String:
        return QString(QLatin1String("\"%1\"")).arg(value.toString());
    case QJsonValue::Array:
        return QLatin1String("[...]");
    case QJsonValue::Object:
        return QLatin1String("{...}");
    case QJsonValue::Undefined:
        return QLatin1String("undefined");
    }
    Q_UNREACHABLE();
    return QString();
}

std::unique_ptr<World> WorldManager::privateLoadWorld(const QString &fileName,
                                                      QString *errorString)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorString)
            *errorString = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    QJsonParseError error;
    const QJsonObject object = QJsonDocument::fromJson(file.readAll(), &error).object();
    if (error.error != QJsonParseError::NoError) {
        if (errorString)
            *errorString = tr("JSON parse error at offset %1:\n%2.").arg(error.offset).arg(error.errorString());
        return nullptr;
    }

    QDir dir = QFileInfo(fileName).dir();
    std::unique_ptr<World> world(new World);

    world->fileName = QFileInfo(fileName).canonicalFilePath();

    const QJsonArray maps = object.value(QLatin1String("maps")).toArray();
    for (const QJsonValue &value : maps) {
        const QJsonObject mapObject = value.toObject();

        World::MapEntry map;
        map.fileName = QDir::cleanPath(dir.filePath(mapObject.value(QLatin1String("fileName")).toString()));
        map.rect = QRect(mapObject.value(QLatin1String("x")).toInt(),
                         mapObject.value(QLatin1String("y")).toInt(),
                         mapObject.value(QLatin1String("width")).toInt(),
                         mapObject.value(QLatin1String("height")).toInt());

        world->maps.append(map);
    }

    const QJsonArray patterns = object.value(QLatin1String("patterns")).toArray();
    for (const QJsonValue &value : patterns) {
        const QJsonObject patternObject = value.toObject();

        World::Pattern pattern;
        pattern.regexp.setPattern(patternObject.value(QLatin1String("regexp")).toString());
        pattern.multiplierX = patternObject.value(QLatin1String("multiplierX")).toInt(1);
        pattern.multiplierY = patternObject.value(QLatin1String("multiplierY")).toInt(1);
        pattern.offset = QPoint(patternObject.value(QLatin1String("offsetX")).toInt(),
                                patternObject.value(QLatin1String("offsetY")).toInt());
        pattern.mapSize = QSize(patternObject.value(QLatin1String("mapWidth")).toInt(std::abs(pattern.multiplierX)),
                                patternObject.value(QLatin1String("mapHeight")).toInt(std::abs(pattern.multiplierY)));

        if (pattern.regexp.captureCount() != 2)
            world->error(tr("World: Invalid number of captures in '%1', 2 captures expected").arg(pattern.regexp.pattern()));
        else if (pattern.multiplierX == 0)
            world->error(tr("World: Invalid multiplierX: %1").arg(jsonValueToString(patternObject.value(QLatin1String("multiplierX")))));
        else if (pattern.multiplierY == 0)
            world->error(tr("World: Invalid multiplierY: %1").arg(jsonValueToString(patternObject.value(QLatin1String("multiplierY")))));
        else if (pattern.mapSize.width() <= 0)
            world->error(tr("World: Invalid mapWidth: %1").arg(jsonValueToString(patternObject.value(QLatin1String("mapWidth")))));
        else if (pattern.mapSize.height() <= 0)
            world->error(tr("World: Invalid mapHeight: %1").arg(jsonValueToString(patternObject.value(QLatin1String("mapHeight")))));
        else
            world->patterns.append(pattern);
    }

    world->onlyShowAdjacentMaps = object.value(QLatin1String("onlyShowAdjacentMaps")).toBool();

    if (world->maps.isEmpty() && world->patterns.isEmpty())
        world->warning(tr("World contained no valid maps or patterns: %1").arg(fileName));

    return world;
}

/**
 * Loads the world with the given \a fileName.
 *
 * \returns the world if it was loaded succesfully, optionally setting
 *          \a errorString when not.
 */
World *WorldManager::loadWorld(const QString &fileName, QString *errorString)
{
    auto world = privateLoadWorld(fileName, errorString);
    if (!world)
        return nullptr;

    if (mWorlds.contains(fileName))
        delete mWorlds.take(fileName);
    else
        mFileSystemWatcher.addPath(fileName);

    mWorlds.insert(fileName, world.release());
    emit worldsChanged();

    return mWorlds.value(fileName);
}

/**
 * Unloads the world with the given \a fileName.
 */
void WorldManager::unloadWorld(const QString &fileName)
{
    std::unique_ptr<World> world { mWorlds.take(fileName) };
    if (world) {
        mFileSystemWatcher.removePath(fileName);
        emit worldsChanged();
    }
}

const World *WorldManager::worldForMap(const QString &fileName) const
{
    for (auto world : mWorlds)
        if (world->containsMap(fileName))
            return world;

    return nullptr;
}

bool World::containsMap(const QString &fileName) const
{
    for (const World::MapEntry &mapEntry : maps) {
        if (mapEntry.fileName == fileName)
            return true;
    }

    for (const World::Pattern &pattern : patterns) {
        QRegularExpressionMatch match = pattern.regexp.match(fileName);
        if (match.hasMatch())
            return true;
    }

    return false;
}

QRect World::mapRect(const QString &fileName) const
{
    for (const World::MapEntry &mapEntry : maps) {
        if (mapEntry.fileName == fileName)
            return mapEntry.rect;
    }

    for (const World::Pattern &pattern : patterns) {
        QRegularExpressionMatch match = pattern.regexp.match(fileName);
        if (match.hasMatch()) {
            const int x = match.capturedRef(1).toInt();
            const int y = match.capturedRef(2).toInt();

            return QRect(QPoint(x * pattern.multiplierX,
                                y * pattern.multiplierY) + pattern.offset,
                         pattern.mapSize);
        }
    }

    return QRect();
}

QVector<World::MapEntry> World::allMaps() const
{
    QVector<World::MapEntry> all(maps);

    if (!patterns.isEmpty()) {
        const QDir dir(QFileInfo(fileName).dir());
        const QStringList entries = dir.entryList(QDir::Files | QDir::Readable);

        for (const World::Pattern &pattern : patterns) {
            for (const QString &fileName : entries) {
                QRegularExpressionMatch match = pattern.regexp.match(fileName);
                if (match.hasMatch()) {
                    const int x = match.capturedRef(1).toInt();
                    const int y = match.capturedRef(2).toInt();

                    MapEntry entry;
                    entry.fileName = dir.filePath(fileName);
                    entry.rect = QRect(QPoint(x * pattern.multiplierX,
                                              y * pattern.multiplierY) + pattern.offset,
                                       pattern.mapSize);
                    all.append(entry);
                }
            }
        }
    }

    return all;
}

QVector<World::MapEntry> World::mapsInRect(const QRect &rect) const
{
    const QVector<World::MapEntry> all(allMaps());

    QVector<World::MapEntry> maps;

    for (const World::MapEntry &mapEntry : all) {
        if (mapEntry.rect.intersects(rect))
            maps.append(mapEntry);
    }

    return maps;
}

QVector<World::MapEntry> World::contextMaps(const QString &fileName) const
{
    if (onlyShowAdjacentMaps)
        return mapsInRect(mapRect(fileName).adjusted(-1, -1, 1, 1));
    return allMaps();
}

void World::error(const QString &message) const
{
    ERROR(message, [fileName = this->fileName] { QDesktopServices::openUrl(QUrl::fromLocalFile(fileName)); }, this);
}

void World::warning(const QString &message) const
{
    WARNING(message, [fileName = this->fileName] { QDesktopServices::openUrl(QUrl::fromLocalFile(fileName)); }, this);
}

void World::clearErrorsAndWarnings() const
{
    emit LoggingInterface::instance().removeIssuesWithContext(this);
}

} // namespace Tiled
