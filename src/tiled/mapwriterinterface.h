/*
 * mapwriterinterface.h
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

#ifndef MAPWRITERINTERFACE_H
#define MAPWRITERINTERFACE_H

#include <QtPlugin>
#include <QStringList>

class QString;

namespace Tiled {

class Map;

/**
 * An interface to be implemented by map writers. A map writer implements
 * support for saving to a certain map format.
 *
 * Tiled provides a writer for its own .tmx map format through the
 * TmxMapWriter. Other writers can be provided by plugins.
 */
class MapWriterInterface
{
public:
    virtual ~MapWriterInterface() {}

    /**
     * Writes the given map to the given file name.
     *
     * @return <code>true</code> on success, <code>false</code> when an error
     *         occurred. The error can be retrieved by errorString().
     */
    virtual bool write(const Map *map, const QString &fileName) = 0;

    /**
     * Returns name filters of this map writer, for multiple formats.
     */
    virtual QStringList nameFilters() const { return QStringList(nameFilter()); }

    /**
     * Returns the error to be shown to the user if an error occured while
     * trying to write a map.
     */
    virtual QString errorString() const = 0;

protected:
    /**
     * Returns the name filter of this map writer.
     *
     * Protected because it should not be used outside the plugin since
     * plugins may expose multiple name filters. This thing exists only for
     * convenience for plugin writers since most plugins will have only one
     * name filter.
     */
    virtual QString nameFilter() const { return QString(); }
};

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::MapWriterInterface,
                    "org.mapeditor.MapWriterInterface")

#endif // MAPWRITERINTERFACE_H
