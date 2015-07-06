/*
 * CSV Tiled Plugin
 * Copyright 2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef CSVPLUGIN_H
#define CSVPLUGIN_H

#include "mapwriterinterface.h"

#include "csv_global.h"

namespace Csv {

class CSVSHARED_EXPORT CsvPlugin : public QObject,
                                   public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapWriterInterface)
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapWriterInterface" FILE "plugin.json")

public:
    CsvPlugin();

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);
    QString errorString() const;
    QStringList outputFiles(const Tiled::Map *map, const QString &fileName) const;

protected:
    QString nameFilter() const;

private:
    QString mError;
};

} // namespace Csv

#endif // CSVPLUGIN_H
