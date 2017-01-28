/*
 * Orx Engine Tiled Plugin
 * Copyright 2017, Denis Brachet aka Ainvar <thegwydd@gmail.com>
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

#ifndef ORX_PLUGIN_H
#define ORX_PLUGIN_H

#include "orx_global.h"

#include "mapformat.h"

#include <QObject>

namespace Orx {

class ORX_SHARED_EXPORT OrxPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    OrxPlugin();

    bool write(const Tiled::Map *map, const QString &fileName) override;
    QString nameFilter() const override;
    QString errorString() const override;

private:
    QString mError;
};

} // namespace Orx

#endif // ORX_PLUGIN_H
