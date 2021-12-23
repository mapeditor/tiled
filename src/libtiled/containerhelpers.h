/*
 *
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <algorithm>
#include <vector>

/*
 * These helper functions are necessary with Qt 5, because the QList::indexOf
 * and QList::contains functions require the argument to be of exactly the
 * contained type. In Qt 6, they can be different types and just need to be
 * comparable.
 */

template<typename Container, typename Value>
inline int indexOf(const Container &container, Value value)
{
    auto it = std::find(container.begin(), container.end(), value);
    return it == container.end() ? -1 : std::distance(container.begin(), it);
}

template<typename Container, typename Value>
inline bool contains(const Container &container, Value value)
{
    return std::find(container.begin(),
                     container.end(),
                     value) != container.end();
}

template<typename Container, typename Pred>
inline bool contains_where(const Container &container, Pred pred)
{
    return std::find_if(container.begin(),
                        container.end(),
                        pred) != container.end();
}

// Based on QVector::move, for use with std::vector
template<typename Value>
void move(std::vector<Value> &container, size_t from, size_t to)
{
    const auto b = container.begin();
    if (from < to)
        std::rotate(b + from, b + from + 1, b + to + 1);
    else
        std::rotate(b + to, b + from, b + from + 1);
}
