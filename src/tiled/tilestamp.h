/*
 * tilestamp.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "map.h"
#include "tiled.h"

#include <QDir>
#include <QJsonObject>
#include <QSharedData>
#include <QString>
#include <QVector>

namespace Tiled {
namespace Internal {

struct TileStampVariation
{
    TileStampVariation()
        : map(nullptr), probability(1.0)
    {
    }

    TileStampVariation(Map *map, qreal probability = 1.0)
        : map(map), probability(probability)
    {
        Q_ASSERT(map->layerCount() >= 1);
        Q_ASSERT(map->layerAt(0)->isTileLayer());
    }

    TileLayer *tileLayer() const;

    Map *map;
    qreal probability;
};

class TileStampData;


class TileStamp
{
public:
    TileStamp();
    explicit TileStamp(Map *map);

    TileStamp(const TileStamp &other);
    TileStamp &operator=(const TileStamp &other);

    bool operator==(const TileStamp &other) const;

    ~TileStamp();

    QString name() const;
    void setName(const QString &name);

    QString fileName() const;
    void setFileName(const QString &fileName);

    qreal probability(int index) const;
    void setProbability(int index, qreal probability);

    QSize maxSize() const;

    const QVector<TileStampVariation> &variations() const;
    void addVariation(Map *map, qreal probability = 1.0);
    void addVariation(const TileStampVariation &variation);
    Map *takeVariation(int index);
    void deleteVariation(int index);
    bool isEmpty() const;

    int quickStampIndex() const;
    void setQuickStampIndex(int quickStampIndex);

    TileStampVariation randomVariation() const;

    TileStamp flipped(FlipDirection direction) const;
    TileStamp rotated(RotateDirection direction) const;

    TileStamp clone() const;

    QJsonObject toJson(const QDir &dir) const;

    static TileStamp fromJson(const QJsonObject &json,
                              const QDir &mapDir);

private:
    QExplicitlySharedDataPointer<TileStampData> d;
};


/**
 * Adds a \a variation to this tile stamp.
 */
inline void TileStamp::addVariation(const TileStampVariation &variation)
{
    addVariation(new Map(*variation.map), variation.probability);
}

} // namespace Internal
} // namespace Tiled
