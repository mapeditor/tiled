/*
 * grid.h
 * Copyright 2020, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QHash>
#include <QPoint>
#include <QRect>

namespace Tiled {

/**
 * An infinite grid of values.
 *
 * The values are stored in chunks that are allocated on-demand. The idea is to
 * be more efficient than a simple QHash<QPoint, T> due to the memory-locality
 * of neighboring values.
 */
template <typename T, int CHUNK_BITS = 4>
class Grid
{
public:
    static const int CHUNK_SIZE = 1 << CHUNK_BITS;
    static const int CHUNK_MASK = CHUNK_SIZE - 1;

    /**
     * A Chunk is a CHUNK_SIZE x CHUNK_SIZE piece of the grid.
     */
    class Chunk
    {
    public:
        Chunk() :
            mValues(CHUNK_SIZE * CHUNK_SIZE)
        {}

        T &get(int x, int y) { return mValues[x + y * CHUNK_SIZE]; }
        T &get(QPoint point) { return get(point.x(), point.y()); }

        const T &get(int x, int y) const { return mValues.at(x + y * CHUNK_SIZE); }
        const T &get(QPoint point) const { return get(point.x(), point.y()); }

        void set(int x, int y, const T &value) { mValues[x + y * CHUNK_SIZE] = value; }
        void set(QPoint point, const T &value) { set(point.x(), point.y(), value); }

        bool isEmpty() const
        {
            for (const T &value : mValues)
                if (value != T())
                    return false;

            return true;
        }

        typename QVector<T>::iterator begin() { return mValues.begin(); }
        typename QVector<T>::iterator end() { return mValues.end(); }
        typename QVector<T>::const_iterator begin() const { return mValues.begin(); }
        typename QVector<T>::const_iterator end() const { return mValues.end(); }

    private:
        QVector<T> mValues;
    };

    /**
     * Returns a read-only reference to the value at the given coordinates.
     */
    const T &get(int x, int y) const
    {
        static const T EMPTY;

        if (const Chunk *chunk = findChunk(x, y))
            return chunk->get(x & CHUNK_MASK, y & CHUNK_MASK);
        else
            return EMPTY;
    }

    const T &get(QPoint point) const
    {
        return get(point.x(), point.y());
    }

    T &add(int x, int y)
    {
        Chunk *chunk = findChunk(x, y);
        if (!chunk)
            chunk = &this->chunk(x, y);

        return chunk->get(x & CHUNK_MASK, y & CHUNK_MASK);
    }

    T &add(QPoint point)
    {
        return add(point.x(), point.y());
    }

    /**
     * Sets the value at the given coordinates.
     */
    void set(int x, int y, const T &value)
    {
        Chunk *chunk = findChunk(x, y);

        if (!chunk) {
            if (value == T())
                return;
            else
                chunk = &this->chunk(x, y);
        }

        chunk->set(x & CHUNK_MASK, y & CHUNK_MASK, value);
    }

    void set(QPoint point, const T &value)
    {
        set(point.x(), point.y(), value);
    }

    /**
     * Returns whether all values in this grid are empty.
     */
    bool isEmpty() const
    {
        for (const Chunk &chunk : mChunks)
            if (!chunk.isEmpty())
                return false;

        return true;
    }

    /**
     * Returns the bounding rect of the allocated chunks.
     */
    QRect bounds() const
    {
        QRect bounds;
        for (auto it = mChunks.keyBegin(); it != mChunks.keyEnd(); ++it)
            bounds |= QRect(it->x(), it->y(), CHUNK_SIZE, CHUNK_SIZE);
        return bounds;
    }

private:
    Chunk &chunk(int x, int y)
    {
        return mChunks[QPoint(x >> CHUNK_BITS, y >> CHUNK_BITS)];
    }

    Chunk *findChunk(int x, int y)
    {
        auto it = mChunks.find(QPoint(x >> CHUNK_BITS, y >> CHUNK_BITS));
        return it != mChunks.end() ? &it.value() : nullptr;
    }

    const Chunk *findChunk(int x, int y) const
    {
        auto it = mChunks.find(QPoint(x >> CHUNK_BITS, y >> CHUNK_BITS));
        return it != mChunks.end() ? &it.value() : nullptr;
    }

    QHash<QPoint, Chunk> mChunks;
};

} // namespace Tiled
