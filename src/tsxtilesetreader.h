/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef TSXTILESETREADER_H
#define TSXTILESETREADER_H

#include <QString>

namespace Tiled {

class Tileset;

namespace Internal {

/**
 * A reader for the .tsx tileset format. Used by the TmxMapReader.
 *
 * This class is also a test to see whether QXmlStreamReader is more convenient
 * to use than QXmlSimpleReader.
 */
class TsxTilesetReader
{
public:
    /**
     * Constructor.
     */
    TsxTilesetReader();

    /**
     * Reads the given tileset file.
     *
     * @return the loaded tileset, or 0 when an error occurred.
     */
    Tileset *readTileset(const QString &fileName);

    /**
     * Returns the error string. This is set when readTileset returned 0.
     */
    QString errorString() const;

private:
    QString mError;
};

} // namespace Internal
} // namespace Tiled

#endif // TSXTILESETREADER_H
