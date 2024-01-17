/*
 * world.h
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

#pragma once

#include "object.h"

#include <QCoreApplication>
#include <QPoint>
#include <QRect>
#include <QRegularExpression>
#include <QSize>
#include <QVector>

#include <memory>

namespace Tiled {

class TILEDSHARED_EXPORT WorldMapEntry
{
    Q_GADGET
    Q_PROPERTY(QString fileName MEMBER fileName)
    Q_PROPERTY(QRect rect MEMBER rect)

public:
    QString fileName;
    QRect rect;
};

class TILEDSHARED_EXPORT WorldPattern
{
    Q_GADGET
    Q_PROPERTY(QRegularExpression regExp MEMBER regexp);
    Q_PROPERTY(int multiplierX MEMBER multiplierX);
    Q_PROPERTY(int multiplierY MEMBER multiplierY);
    Q_PROPERTY(QPoint offset MEMBER offset);
    Q_PROPERTY(QSize mapSize MEMBER mapSize);

public:
    QRegularExpression regexp;
    int multiplierX;
    int multiplierY;
    QPoint offset;
    QSize mapSize;
};

class TILEDSHARED_EXPORT World : public Object
{
    Q_DECLARE_TR_FUNCTIONS(Tiled::WorldManager);

public:
    World() : Object(WorldType) {}

    QString fileName;
    QVector<WorldMapEntry> maps;
    QVector<WorldPattern> patterns;
    bool onlyShowAdjacentMaps = false;
    bool hasUnsavedChanges = false;

    int mapIndex(const QString &fileName) const;
    void setMapRect(int mapIndex, const QRect &rect);
    void addMap(const QString &fileName, const QRect &rect);
    void removeMap(int mapIndex);
    bool containsMap(const QString &fileName) const;
    QRect mapRect(const QString &fileName) const;
    QVector<WorldMapEntry> allMaps() const;
    QVector<WorldMapEntry> mapsInRect(const QRect &rect) const;
    QVector<WorldMapEntry> contextMaps(const QString &fileName) const;
    QString firstMap() const;

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

    static std::unique_ptr<World> load(const QString &fileName,
                                       QString *errorString = nullptr);
    static bool save(World &world,
                     QString *errorString = nullptr);
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::WorldPattern)
Q_DECLARE_METATYPE(Tiled::WorldMapEntry)
