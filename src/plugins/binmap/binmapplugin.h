/*
 * BinMap Tiled Plugin
 * Copyright 2025, bzt <bztsrc@gitlab>
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

#include "binmap_global.h"

#include "mapformat.h"

#include <QMap>
#include <QObject>
#include <QTextStream>

namespace Binmap {

class BINMAPSHARED_EXPORT BinmapPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    BinmapPlugin();

    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;
    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

private:
    QString mError;
};

} // namespace Binmap
