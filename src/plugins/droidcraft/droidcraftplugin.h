/*
 * Droidcraft Tiled Plugin
 * Copyright 2011, seeseekey <seeseekey@googlemail.com>
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

#ifndef DROIDCRAFTPLUGIN_H
#define DROIDCRAFTPLUGIN_H

#include "droidcraft_global.h"

#include "map.h"
#include "mapwriterinterface.h"
#include "mapreaderinterface.h"

#include <QObject>

namespace Droidcraft {

class DROIDCRAFTSHARED_EXPORT DroidcraftPlugin :
        public QObject,
        public Tiled::MapWriterInterface,
        public Tiled::MapReaderInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapReaderInterface)
    Q_INTERFACES(Tiled::MapWriterInterface)

public:
    DroidcraftPlugin();

    // MapReaderInterface
    Tiled::Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
    QString errorString() const;

private:
    QString mError;
};

} // namespace Droidcraft

#endif // DROIDCRAFTPLUGIN_H
