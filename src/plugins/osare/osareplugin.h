/*
 * Osare Tiled Plugin
 * Copyright Jaderamiso
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef OSAREPLUGIN_H
#define OSAREPLUGIN_H
#include "osare_global.h"

#include "mapwriterinterface.h"

#include <QObject>
#include <QMap>
#include "tileset.h"

namespace Osare {

class OSARESHARED_EXPORT OsarePlugin :
        public QObject,
        public Tiled::MapWriterInterface
    {
    Q_OBJECT
    Q_INTERFACES(Tiled::MapWriterInterface)

    public:
        OsarePlugin();

        // MapWriterInterface
        bool write(const Tiled::Map *map, const QString &fileName);
        QString nameFilter() const;
        QString errorString() const;

    private:
        QString mError;
    };

} // namespace Osare

#endif // OSAREPLUGIN_H
