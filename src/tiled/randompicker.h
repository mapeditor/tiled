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

#include <random>

namespace Tiled {

inline std::default_random_engine &globalRandomEngine()
{
    static std::default_random_engine engine(std::random_device{}());
    return engine;
}

/**
 * A class that helps pick random things that each have a probability
 * assigned.
 */
template<typename T, typename Real = qreal>
class RandomPicker
{
public:
    RandomPicker()
        : mSum(0.0)
    {}

    void add(const T &value, Real probability = 1.0)
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

        std::uniform_real_distribution<Real> dis(0, mSum);
        const Real random = dis(globalRandomEngine());
        const auto it = mThresholds.lowerBound(random);
        if (it != mThresholds.end())
            return it.value();
        else
            return (mThresholds.end() - 1).value();
    }

    //same as pick, but removes the selected element.
    T take()
    {
        Q_ASSERT(!isEmpty());

        std::uniform_real_distribution<Real> dis(0, mSum);
        const Real random = dis(globalRandomEngine());
        const auto it = mThresholds.lowerBound(random);

        if (it != mThresholds.end())
            return mThresholds.take(it.key());
        else
            return mThresholds.take((it - 1).key());
    }

    void clear()
    {
        mSum = 0.0;
        mThresholds.clear();
    }

private:
    Real mSum;
    QMap<Real, T> mThresholds;
};

} // namespace Tiled
