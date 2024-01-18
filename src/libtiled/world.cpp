/*
 * world.cpp
 * Copyright 2017-2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "world.h"

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

namespace Tiled {

void World::setMapRect(int mapIndex, const QRect &rect)
{
    if (maps[mapIndex].rect != rect) {
        maps[mapIndex].rect = rect;
        hasUnsavedChanges = true;
    }
}

void World::removeMap(int mapIndex)
{
    maps.removeAt(mapIndex);
}

void World::addMap(const QString &fileName, const QRect &rect)
{
    WorldMapEntry entry;
    entry.rect = rect;
    entry.fileName = fileName;
    maps.append(entry);
}

int World::mapIndex(const QString &fileName) const
{
    for (int i = 0; i < maps.length(); i++) {
        if (maps[i].fileName == fileName)
            return i;
    }
    return -1;
}

bool World::containsMap(const QString &fileName) const
{
    for (const WorldMapEntry &mapEntry : maps) {
        if (mapEntry.fileName == fileName)
            return true;
    }

    // Currently patterns can only be used to search for maps in the same
    // folder as the .world file. It could be useful to support a "prefix" or
    // "folders" property per pattern to allow referring to other folders.
    if (QFileInfo(this->fileName).path() != QFileInfo(fileName).path())
        return false;

    for (const WorldPattern &pattern : patterns) {
        QRegularExpressionMatch match = pattern.regexp.match(fileName);
        if (match.hasMatch())
            return true;
    }

    return false;
}

QRect World::mapRect(const QString &fileName) const
{
    for (const WorldMapEntry &mapEntry : maps) {
       if (mapEntry.fileName == fileName)
            return mapEntry.rect;
    }

    for (const WorldPattern &pattern : patterns) {
        QRegularExpressionMatch match = pattern.regexp.match(fileName);
        if (match.hasMatch()) {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            const int x = match.capturedView(1).toInt();
            const int y = match.capturedView(2).toInt();
#else
            const int x = match.capturedRef(1).toInt();
            const int y = match.capturedRef(2).toInt();
#endif

            return QRect(QPoint(x * pattern.multiplierX,
                                y * pattern.multiplierY) + pattern.offset,
                         pattern.mapSize);
        }
    }

    return QRect();
}

QVector<WorldMapEntry> World::allMaps() const
{
    QVector<WorldMapEntry> all(maps);

    if (!patterns.isEmpty()) {
        const QDir dir(QFileInfo(fileName).dir());
        const QStringList entries = dir.entryList(QDir::Files | QDir::Readable);

        for (const WorldPattern &pattern : patterns) {
            for (const QString &fileName : entries) {
                QRegularExpressionMatch match = pattern.regexp.match(fileName);
                if (match.hasMatch()) {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                    const int x = match.capturedView(1).toInt();
                    const int y = match.capturedView(2).toInt();
#else
                    const int x = match.capturedRef(1).toInt();
                    const int y = match.capturedRef(2).toInt();
#endif

                    WorldMapEntry entry;
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

QVector<WorldMapEntry> World::mapsInRect(const QRect &rect) const
{
    QVector<WorldMapEntry> maps(allMaps());

    maps.erase(std::remove_if(maps.begin(), maps.end(),
                              [&](const WorldMapEntry &mapEntry) { return !mapEntry.rect.intersects(rect); }),
               maps.end());

    return maps;
}

QVector<WorldMapEntry> World::contextMaps(const QString &fileName) const
{
    if (onlyShowAdjacentMaps)
        return mapsInRect(mapRect(fileName).adjusted(-1, -1, 1, 1));
    return allMaps();
}

QString World::firstMap() const
{
    if (!maps.isEmpty())
        return maps.first().fileName;

    if (!patterns.isEmpty()) {
        const QDir dir(QFileInfo(fileName).dir());
        const QStringList entries = dir.entryList(QDir::Files | QDir::Readable);

        for (const WorldPattern &pattern : patterns) {
            for (const QString &fileName : entries) {
                QRegularExpressionMatch match = pattern.regexp.match(fileName);
                if (match.hasMatch())
                    return dir.filePath(fileName);
            }
        }
    }

    return QString();
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


bool World::canBeModified() const
{
    return patterns.isEmpty();
}

QString World::displayName() const
{
    return displayName(fileName);
}

QString World::displayName(const QString &fileName)
{
    return QFileInfo(fileName).fileName();
}

static QString jsonValueToString(const QJsonValue &value)
{
    switch (value.type()) {
    case QJsonValue::Null:
        return QStringLiteral("null");
    case QJsonValue::Bool:
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    case QJsonValue::Double:
        return QString::number(value.toDouble());
    case QJsonValue::String:
        return QStringLiteral("\"%1\"").arg(value.toString());
    case QJsonValue::Array:
        return QStringLiteral("[...]");
    case QJsonValue::Object:
        return QStringLiteral("{...}");
    case QJsonValue::Undefined:
        return QStringLiteral("undefined");
    }
    Q_UNREACHABLE();
    return QString();
}


std::unique_ptr<World> World::load(const QString &fileName,
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

        WorldMapEntry map;
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

        WorldPattern pattern;
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

bool World::save(World &world, QString *errorString)
{
    const QDir worldDir = QFileInfo(world.fileName).dir();

    QJsonArray maps;
    for (const WorldMapEntry &map : std::as_const(world.maps)) {
        QJsonObject jsonMap;

        const QString relativeFileName = QDir::cleanPath(worldDir.relativeFilePath(map.fileName));

        jsonMap.insert(QLatin1String("fileName"), relativeFileName);
        jsonMap.insert(QLatin1String("x"), map.rect.x());
        jsonMap.insert(QLatin1String("y"), map.rect.y());
        jsonMap.insert(QLatin1String("width"), map.rect.width());
        jsonMap.insert(QLatin1String("height"), map.rect.height());
        maps.append(jsonMap);
    }

    QJsonArray patterns;
    for (const WorldPattern &pattern : std::as_const(world.patterns)) {
        QJsonObject jsonPattern;

        jsonPattern.insert(QLatin1String("regexp"), pattern.regexp.pattern());
        if (pattern.multiplierX != 1)
            jsonPattern.insert(QLatin1String("multiplierX"), pattern.multiplierX);
        if (pattern.multiplierY != 1)
            jsonPattern.insert(QLatin1String("multiplierY"), pattern.multiplierY);
        if (pattern.offset.x() != 0)
            jsonPattern.insert(QLatin1String("offsetX"), pattern.offset.x());
        if (pattern.offset.y() != 0)
            jsonPattern.insert(QLatin1String("offsetY"), pattern.offset.y());
        if (pattern.mapSize.width() != std::abs(pattern.multiplierX))
            jsonPattern.insert(QLatin1String("mapWidth"), pattern.mapSize.width());
        if (pattern.mapSize.height() != std::abs(pattern.multiplierY))
            jsonPattern.insert(QLatin1String("mapHeight"), pattern.mapSize.height());
        patterns.append(jsonPattern);
    }

    QJsonObject document;
    if (!maps.isEmpty())
        document.insert(QLatin1String("maps"), maps);
    if (!patterns.isEmpty())
        document.insert(QLatin1String("patterns"), patterns);
    document.insert(QLatin1String("type"), QLatin1String("world"));
    document.insert(QLatin1String("onlyShowAdjacentMaps"), world.onlyShowAdjacentMaps);

    QJsonDocument doc(document);

    QFile file(world.fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorString)
            *errorString = tr("Could not open file for reading.");
        return false;
    }

    file.write(doc.toJson());
    file.close();

    world.hasUnsavedChanges = false;

    return true;
}

} // namespace Tiled

#include "moc_world.cpp"
