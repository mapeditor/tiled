/*
 * The T-Engine 4 Tiled Plugin
 * Copyright 2010, Mikolai Fajer <mfajer@gmail.com>
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

#ifndef TENGINEPLUGIN_H
#define TENGINEPLUGIN_H

#include "tengine_global.h"

#include "mapwriterinterface.h"
#include "properties.h"

#include <QObject>

namespace Tengine {
// ASCII characters between decimals 32 and 126 should be ok
const int ASCII_MIN = 32;
const int ASCII_MAX = 126;

class TENGINESHARED_EXPORT TenginePlugin : public QObject,
                                   public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapWriterInterface)

public:
    TenginePlugin();

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
    QString errorString() const;

private:
    QString mError;
    QString constructArgs(Tiled::Properties props, QList<QString> propOrder) const;
    QString constructAdditionalTable(Tiled::Properties props, QList<QString> propOrder) const;
};

} // namespace Tengine

#endif // TENGINEPLUGIN_H
