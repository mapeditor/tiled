/*
 * randompicker.h
 * Copyright 2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QMap>

namespace Tiled {
namespace Internal {

/**
 * A class that helps pick random things that each have a probability
 * assigned.
 */
template<typename T>
class RandomPicker
{
public:
    RandomPicker()
        : mSum(0.0)
    {}

    void add(const T &value, qreal probability = 1.0)
    {
        if (probability > 0) {
            mSum += probability;
            mThresholds.insert(mSum, value);
        }
    }

    bool isEmpty() const
    {
        return mThresholds.isEmpty();
    }

    const T &pick() const
    {
        Q_ASSERT(!isEmpty());

        const qreal random = ((qreal)rand() / RAND_MAX) * mSum;
        const auto it = mThresholds.lowerBound(random);
        if (it != mThresholds.end())
            return it.value();
        else
            return (mThresholds.end() - 1).value();
    }

    void clear()
    {
        mSum = 0.0;
        mThresholds.clear();
    }

private:
    qreal mSum;
    QMap<qreal, T> mThresholds;
};

} // namespace Internal
} // namespace Tiled
