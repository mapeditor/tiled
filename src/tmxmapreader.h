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

#include "mapreaderinterface.h"

#ifndef TMXMAPREADER_H
#define TMXMAPREADER_H

#include <QObject>
#include <QString>

namespace Tiled {
namespace Internal {

/**
 * A reader for Tiled's .tmx map format.
 */
class TmxMapReader : public MapReaderInterface
{
public:
    Map *read(const QString &fileName);

    /**
     * Reads the map given by \a string. This is for retrieving a map from the
     * clipboard. Returns 0 on failure.
     *
     * @see TmxMapWriter::toString
     */
    Map *fromString(const QString &string);

    QString name() const { return QObject::tr("XML map reader (*.tmx)"); }

    QString errorString() const { return mError; }

private:
    QString mError;
};

} // namespace Internal
} // namespace Tiled

#endif // TMXMAPREADER_H
