/*
 * worldmanager.h
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

#pragma once

#include "tiled_global.h"

#include "filesystemwatcher.h"

#include <QCoreApplication>
#include <QMap>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QRegularExpression>
#include <QSize>
#include <QVector>

#include <memory>

namespace Tiled {

struct TILEDSHARED_EXPORT World
{
    struct Pattern
    {
        QRegularExpression regexp;
        int multiplierX;
        int multiplierY;
        QPoint offset;
        QSize mapSize;
    };

    struct MapEntry
    {
        QString fileName;
        QRect rect;
    };

    QString fileName;
    QVector<MapEntry> maps;
    QVector<Pattern> patterns;
    bool onlyShowAdjacentMaps;

    int mapIndex(const QString &fileName) const;
    void setMapRect(int mapIndex, const QRect &rect);
    void addMap(const QString &fileName, const QRect &rect);
    void removeMap(int mapIndex);
    bool containsMap(const QString &fileName) const;
    QRect mapRect(const QString &fileName) const;
    QVector<MapEntry> allMaps() const;
    QVector<MapEntry> mapsInRect(const QRect &rect) const;
    QVector<MapEntry> contextMaps(const QString &fileName) const;

    void error(const QString &message) const;
    void warning(const QString &message) const;
    void clearErrorsAndWarnings() const;

    bool canBeModified() const;

    /**
     * Returns the name with which to display this world. It is the file name without
     * its path.
     */
    QString displayName() const;
    static QString displayName(const QString &fileName);
};

class TILEDSHARED_EXPORT WorldManager : public QObject
{
    Q_OBJECT

    WorldManager();
    ~WorldManager();

public:
    static WorldManager &instance();
    static void deleteInstance();

    World *addEmptyWorld(const QString &fileName, QString *errorString);
    World *loadWorld(const QString &fileName, QString *errorString = nullptr);
    void loadWorlds(const QStringList &fileNames);
    void unloadWorld(const QString &fileName);
    void unloadAllWorlds();
    bool saveWorld(const QString &fileName, QString *errorString = nullptr);

    const QMap<QString, World*> &worlds() const { return mWorlds; }

    const World *worldForMap(const QString &fileName) const;

    void setMapRect(const QString &fileName, const QRect &rect);
    bool mapCanBeModified(const QString &fileName) const;
    bool removeMap(const QString &fileName);
    bool addMap(const QString &fileName, const QString &mapFileName, const QRect &rect);

signals:
    void worldsChanged();
    void worldReloaded(const QString &fileName);
    void worldUnloaded(const QString &fileName);

private:
    World *loadAndStoreWorld(const QString &fileName, QString *errorString = nullptr);
    void reloadWorldFiles(const QStringList &fileNames);

    std::unique_ptr<World> privateLoadWorld(const QString &fileName,
                                            QString *errorString = nullptr);

    QMap<QString, World*> mWorlds;

    FileSystemWatcher mFileSystemWatcher;
    QString mIgnoreFileChangeEventForFile;

    static WorldManager *mInstance;
};

} // namespace Tiled
