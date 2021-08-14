/*
 * RpMap Tiled Plugin
 * Copyright 2020, Christof Petig <christof.petig@arcor.de>
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

#include "rpmap_global.h"

#include "mapformat.h"

#include <QObject>
#include <QMap>
#include <QXmlStreamWriter>

namespace RpMap {

class RPMAPSHARED_EXPORT RpMapPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    RpMapPlugin();

#if 0
    std::unique_ptr<Tiled::Map> read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;
#endif

    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;
    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

private:
    QString mError;

    QMap<QString, QString> filename2md5;
    QVector<uint32_t> first_used_md5;
    uint32_t number_of_tiles;

    void writeTokenMap(QXmlStreamWriter &writer, Tiled::Map const* map);
    void writeTokenOrderedList(QXmlStreamWriter &writer);
    void writeMap(QXmlStreamWriter &writer, Tiled::Map const* map);
};

} // namespace RpMap
