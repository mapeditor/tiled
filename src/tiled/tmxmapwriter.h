/*
 * tmxmapwriter.h
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

#ifndef TMXMAPWRITER_H
#define TMXMAPWRITER_H

#include "mapwriterinterface.h"

#include <QCoreApplication>
#include <QString>

namespace Tiled {

class Tileset;

namespace Internal {

/**
 * A writer for Tiled's .tmx map format.
 */
class TmxMapWriter : public MapWriterInterface
{
    Q_DECLARE_TR_FUNCTIONS(TmxMapReader)

public:
    bool write(const Map *map, const QString &fileName);

    bool writeTileset(const Tileset *tileset, const QString &fileName);

    /**
     * Converts the given map to a utf8 byte array (in .tmx format). This is
     * for storing a map in the clipboard. References to other files (like
     * tileset images) will be saved as absolute paths.
     *
     * @see TmxMapReader::fromByteArray
     */
    QByteArray toByteArray(const Map *map);

    QString nameFilter() const { return tr("Tiled map files (*.tmx)"); }

    QString errorString() const { return mError; }

private:
    QString mError;
};

} // namespace Internal
} // namespace Tiled

#endif // TMXMAPWRITER_H
