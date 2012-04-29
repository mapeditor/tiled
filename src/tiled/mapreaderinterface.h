/*
 * mapreaderinterface.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MAPREADERINTERFACE_H
#define MAPREADERINTERFACE_H

#include <QtPlugin>
#include <QStringList>

class QString;

namespace Tiled {

class Map;

/**
 * An interface to be implemented by map readers. A map reader implements
 * support for loading a certain map format.
 *
 * At the moment, Tiled only provides a reader for its own .tmx map format
 * through the TmxMapReader.
 */
class MapReaderInterface
{
public:
    virtual ~MapReaderInterface() {}

    /**
     * Reads the map and returns a new Map instance, or 0 if reading failed.
     */
    virtual Map *read(const QString &fileName) = 0;

    /**
     * Returns name filters of this map reader, for multiple formats.
     */
    virtual QStringList nameFilters() const { return QStringList(nameFilter()); }

    /**
     * Returns whether this map reader supports reading the given file.
     *
     * Generally would do a file extension check.
     */
    virtual bool supportsFile(const QString &fileName) const = 0;

    /**
     * Returns the error to be shown to the user if an error occured while
     * trying to read a map.
     */
    virtual QString errorString() const = 0;

protected:
    /**
     * Returns the name filter of this map reader.
     *
     * Protected because it should not be used outside the plugin since
     * plugins may expose multiple name filters. This thing exists only for
     * convenience for plugin writers since most plugins will have only one
     * name filter.
     */
    virtual QString nameFilter() const { return QString(); }
};

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::MapReaderInterface,
                    "org.mapeditor.MapReaderInterface")

#endif // MAPREADERINTERFACE_H
