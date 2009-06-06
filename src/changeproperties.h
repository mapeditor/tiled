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

#ifndef CHANGEPROPERTIES_H
#define CHANGEPROPERTIES_H

#include <QMap>
#include <QString>
#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class ChangeProperties : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Change Properties' command.
     *
     * @param kind          the kind of properties (Map, Layer, Object, etc.)
     * @param properties    the properties instance that should be changed
     * @param newProperties the new properties that should be applied
     */
    ChangeProperties(const QString &kind,
                     QMap<QString, QString> *properties,
                     const QMap<QString, QString> &newProperties);
    void undo();
    void redo();

private:
    void swapProperties();

    QMap<QString, QString> *mProperties;
    QMap<QString, QString> mNewProperties;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEPROPERTIES_H
