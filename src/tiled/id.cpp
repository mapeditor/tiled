/*
 * id.cpp
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

/**
 * The following code is largely based on id.cpp from Qt Creator.
 */

#include "id.h"

#include <QHash>

namespace Tiled {

class StringHash
{
public:
    StringHash()
        : hash(0)
    {}

    explicit StringHash(const QByteArray &s)
        : string(s)
        , hash(qHash(s))
    {}

    QByteArray string;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    uint hash;
#else
    size_t hash;
#endif
};

static bool operator==(const StringHash &sh1, const StringHash &sh2)
{
    return sh1.string == sh2.string;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
static uint qHash(const StringHash &sh)
#else
static size_t qHash(const StringHash &sh)
#endif
{
    return sh.hash;
}

static QHash<uint, StringHash> stringFromId;
static QHash<StringHash, uint> idFromString;


Id::Id(const char *name)
{
    static uint firstUnusedId = 1;

    static QByteArray temp;
    temp.setRawData(name, qstrlen(name));                           // avoid copying data

    if (temp.isEmpty()) {
        mId = 0;
        return;
    }

    StringHash sh(temp);

    uint id = idFromString.value(sh, 0);

    if (id == 0) {
        id = firstUnusedId++;
        sh.string = QByteArray(temp.constData(), temp.length());    // copy
        idFromString.insert(sh, id);
        stringFromId.insert(id, sh);
    }

    mId = id;
}

Id::Id(const QByteArray &name)
    : Id(name.constData())
{
}

QByteArray Id::name() const
{
    return stringFromId.value(mId).string;
}

QString Id::toString() const
{
    return QString::fromUtf8(name());
}


QStringList idsToNames(const QList<Id> &ids)
{
    QStringList names;
    names.reserve(ids.size());
    for (const Id &id : ids)
        names.append(id.toString());

    names.sort();

    return names;
}

QList<Id> namesToIds(const QStringList &names)
{
    QList<Id> ids;
    ids.reserve(names.size());
    for (const QString &name : names)
        ids.append(Id(name.toUtf8()));

    return ids;
}

} // namespace Tiled
