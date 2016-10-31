/*
 * id.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#ifndef ID_H
#define ID_H

#include <QLatin1String>

namespace Tiled {
namespace Internal {

class Id
{
public:
    Id(const char *name);

    QByteArray name() const;

    bool operator==(Id id) const { return mId == id.mId; }
    bool operator!=(Id id) const { return mId != id.mId; }

private:
    uint mId;

    friend uint qHash(Id id);
};


inline uint qHash(Id id)
{
    return id.mId;
}

} // namespace Internal
} // namespace Tiled

#endif // ID_H
