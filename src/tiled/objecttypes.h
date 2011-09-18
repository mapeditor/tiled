/*
 * objecttypes.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef OBJECTTYPES_H
#define OBJECTTYPES_H

#include <QString>
#include <QColor>
#include <QVector>

namespace Tiled {
namespace Internal {

/**
 * Quick definition of an object type. It has a name and a color.
 */
struct ObjectType
{
    ObjectType() : color(Qt::gray) {}

    ObjectType(const QString &name,
               const QColor &color)
        : name(name)
        , color(color)
    {}

    QString name;
    QColor color;
};

typedef QVector<ObjectType> ObjectTypes;


class ObjectTypesWriter
{
public:
    bool writeObjectTypes(const QString &fileName,
                          const ObjectTypes &objectTypes);

    QString errorString() const { return mError; }

private:
    QString mError;
};


class ObjectTypesReader
{
public:
    ObjectTypes readObjectTypes(const QString &fileName);

    QString errorString() const { return mError; }

private:
    QString mError;
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTTYPES_H
