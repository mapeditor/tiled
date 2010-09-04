/*
 * changeproperties.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef CHANGEPROPERTIES_H
#define CHANGEPROPERTIES_H

#include "properties.h"

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
                     Properties *properties,
                     const Properties &newProperties);
    void undo();
    void redo();

private:
    void swapProperties();

    Properties *mProperties;
    Properties mNewProperties;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEPROPERTIES_H
