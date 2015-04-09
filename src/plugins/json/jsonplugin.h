/*
 * JSON Tiled Plugin
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
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

#ifndef JSONPLUGIN_H
#define JSONPLUGIN_H

#include "json_global.h"

#include "mapwriterinterface.h"
#include "mapreaderinterface.h"

#include <QObject>

namespace Tiled {
class Map;
}

namespace Json {

class JSONSHARED_EXPORT JsonPlugin
        : public QObject
        , public Tiled::MapReaderInterface
        , public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapReaderInterface)
    Q_INTERFACES(Tiled::MapWriterInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapWriterInterface" FILE "plugin.json")
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapReaderInterface" FILE "plugin.json")
#endif

public:
    JsonPlugin();

    // MapReaderInterface
    Tiled::Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);

    // Both interfaces
    QStringList nameFilters() const;
    QString errorString() const;

private:
    QString mError;
};

} // namespace Json

#endif // JSONPLUGIN_H
