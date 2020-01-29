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

#pragma once

#include <QLatin1String>
#include <QMetaType>

namespace Tiled {

class Id
{
public:
    Id() : mId(0) {}
    Id(const char *name);
    Id(const QByteArray &name);

    QByteArray name() const;
    QString toString() const;
    bool isNull() const { return mId == 0; }

    explicit operator bool() const { return !isNull(); }

    bool operator==(Id id) const { return mId == id.mId; }
    bool operator!=(Id id) const { return mId != id.mId; }
    bool operator<(Id id) const { return name() < id.name(); }

private:
    uint mId;

    friend uint qHash(Id id) Q_DECL_NOTHROW;
};


inline uint qHash(Id id) Q_DECL_NOTHROW
{
    return id.mId;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Id)
