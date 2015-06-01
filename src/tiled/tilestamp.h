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

#ifndef TILED_INTERNAL_TILESTAMP_H
#define TILED_INTERNAL_TILESTAMP_H

#include "map.h"

#include <QString>
#include <QVector>

namespace Tiled {
namespace Internal {

struct TileStampVariation
{
    TileStampVariation(Map *map = 0, qreal probability = 1.0)
        : map(map), probability(probability)
    {}

    Map *map;
    qreal probability;
};


class TileStamp
{
public:
    TileStamp();
    ~TileStamp();

    QString name() const;
    void setName(const QString &name);

    qreal probability(int index) const;
    void setProbability(int index, qreal probability);

    const QVector<TileStampVariation> &variations() const;
    void addVariation(Map *map, qreal probability = 1.0);
    Map *takeVariation(int index);

    int quickStampIndex() const;
    void setQuickStampIndex(int quickStampIndex);

    Map *randomVariation() const;

private:
    QString mName;
    QVector<TileStampVariation> mVariations;
    int mQuickStampIndex;
};


inline QString TileStamp::name() const
{
    return mName;
}

inline void TileStamp::setName(const QString &name)
{
    mName = name;
}

inline qreal TileStamp::probability(int index) const
{
    return mVariations.at(index).probability;
}

inline void TileStamp::setProbability(int index, qreal probability)
{
    mVariations[index].probability = probability;
}

inline const QVector<TileStampVariation> &TileStamp::variations() const
{
    return mVariations;
}

inline int TileStamp::quickStampIndex() const
{
    return mQuickStampIndex;
}

inline void TileStamp::setQuickStampIndex(int quickStampIndex)
{
    mQuickStampIndex = quickStampIndex;
}

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILESTAMP_H
