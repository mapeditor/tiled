/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MAPREADERINTERFACE_H
#define MAPREADERINTERFACE_H

class QString;

namespace Tiled {

class Map;

/**
 * An interface to be implemented by map readers. A map reader implements
 * support for loading a certain map format.
 *
 * At the moment, Tiled only provides a reader for its own .tmx map format
 * through the XmlMapReader.
 */
class MapReaderInterface
{
public:
    virtual ~MapReaderInterface() {}

    /**
     * Reads the map and returns a new Map instance, or 0 if reading failed.
     */
    virtual Map* read(const QString &fileName) = 0;

    /**
     * Returns the name of this map reader.
     */
    virtual QString name() const = 0;

    /**
     * Returns the error to be shown to the user if an error occured while
     * trying to read a map.
     */
    virtual QString errorString() const = 0;
};

} // namespace Tiled

#endif // MAPREADERINTERFACE_H
