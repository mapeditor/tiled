/*
 * tmxmapreader.h
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

#ifndef TMXMAPREADER_H
#define TMXMAPREADER_H

#include "mapreaderinterface.h"

#include <QCoreApplication>
#include <QString>

namespace Tiled {

class Tileset;

namespace Internal {

/**
 * A reader for Tiled's .tmx map format.
 */
class TmxMapReader : public MapReaderInterface
{
    Q_DECLARE_TR_FUNCTIONS(TmxMapReader)

public:
    Map *read(const QString &fileName);

    /**
     * Reads the map given by \a data. This is for retrieving a map from the
     * clipboard. Returns 0 on failure.
     *
     * @see TmxMapWriter::toByteArray
     */
    Map *fromByteArray(const QByteArray &data);

    Tileset *readTileset(const QString &fileName);

    QString nameFilter() const { return tr("Tiled map files (*.tmx)"); }

    bool supportsFile(const QString &fileName) const
    { return fileName.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive); }

    QString errorString() const { return mError; }

private:
    QString mError;
};

} // namespace Internal
} // namespace Tiled

#endif // TMXMAPREADER_H
