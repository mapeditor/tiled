/*
 * rangeset.h
 * Copyright 2011, Ben Longbons <b.r.longbons@gmail.com>
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

#ifndef RANGESET_H
#define RANGESET_H

#include <map>

namespace Tiled {
namespace Internal {

/**
 * Logically, a set, but implemented as a set of ranges. This class is only
 * intended to be used with primitive integral types.
 */
template<typename Int>
class RangeSet
{
    // This class is based on std::map rather than QMap since std::map's insert
    // method has an overload that takes a hint about where to insert the new
    // pair.
    typedef typename std::map<Int, Int> Map;
    typedef typename Map::iterator iterator_;
    typedef typename Map::const_iterator const_iterator_;

    Map mMap;

public:
    /**
     * Extends the real iterator type with some convenience methods.
     */
    class Range : public Map::const_iterator
    {
    public:
        Range() {}
        Range(const const_iterator_ &p) : const_iterator_(p) {}

        /**
         * Returns the first value included in this range.
         */
        Int first() const
        { return (*this)->first; }

        /**
         * Returns the last value included in this range.
         */
        Int last() const
        { return (*this)->second; }

        /**
         * Returns the number of values included in this range.
         */
        Int length() const
        { return last() - first() + 1; }
    };

    typedef Range const_iterator;

    /**
     * Insert \a value in the set of ranges. Has no effect when the value is
     * already part of an existing range. When possible, an existing range is
     * extended to include the new value, otherwise a new range is inserted.
     */
    void insert(Int value)
    {
        if (mMap.empty()) {
            mMap.insert(std::make_pair(value, value));
            return;
        }

        // We can now assume that 'it' will be at most one end of the range

        // This is the only full-tree search of the map, everything else is
        // relative to this
        iterator_ it = mMap.lower_bound(value);
        iterator_ begin = mMap.begin();
        iterator_ end = mMap.end();

        if (it == end) {
            // Check whether the value is included in the last range
            // assert: it != begin
            --it;
            // assert: it->first < value
            if (it->second >= value)
                return;

            // Try to add the value to the end of the previous range
            if (++it->second == value)
                return;

            // Didn't work, restore the previous range
            --it->second;

            // We have to insert a new range
            mMap.insert(it, std::make_pair(value, value));
            return;
        }

        // Now we can dereference 'it' itself
        // assert: it->first >= value
        if (it->first == value)
            return;

        // Check whether we can extend the range downwards to include value
        if (it->first == value + 1) {
            // When extending the range downwards, it may need to be merged
            // with the previous range.

            // Remember 'prev' for the insertion hint. It is not necessarily
            // before the value, if it == begin.
            iterator_ prev = it;
            if (it != begin) {
                --prev;
                if (prev->second == value - 1) {
                    // The new value fills the gab. Merge the ranges, leaving
                    // only the first, but with a larger range.
                    prev->second = it->second;
                    mMap.erase(it);
                    return;
                }
            }
            // No merge needed

            // To change the key, we have to both add and remove. Add first,
            // then remove, to avoid invalidating the iterator too early.
            mMap.insert(prev, std::make_pair(value, it->second));
            mMap.erase(it);
            return;
        }

        // Check if we can grow the previous range upwards to include value
        if (it != begin) {
            --it;
            if (it->second == value - 1) {
                ++it->second;
                return;
            }
        }

        // 'it' now points below the range, unless it was already begin
        // We couldn't increase an existing range
        mMap.insert(it, std::make_pair(value, value));
    }

    /**
     * Removes all ranges from this set.
     */
    void clear()
    {
        mMap.clear();
    }

    // Only const iterators are provided, because it is not safe to modify the
    // underlying list. Note that const_iterator is a typedef for Range.

    const_iterator begin() const
    {
        return Range(mMap.begin());
    }

    const_iterator end() const
    {
        return Range(mMap.end());
    }

    bool isEmpty() const
    {
        return mMap.empty();
    }
};

} // namespace Internal
} // namespace Tiled

#endif // RANGESET_H
