/*
 * The Mana World Tiled Plugin
 * Copyright 2010 Thorbjørn Lindeijer
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

#ifndef TMWPLUGIN_H
#define TMWPLUGIN_H

#include "tmw_global.h"

#include "mapwriterinterface.h"

#include <QObject>

namespace Tmw {

class TMWSHARED_EXPORT TmwPlugin : public QObject,
                                   public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapWriterInterface)

public:
    TmwPlugin();

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
    QString errorString() const;

private:
    QString mError;
};

} // namespace Mana

#endif // TMWPLUGIN_H
