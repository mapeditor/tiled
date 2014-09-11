/*
 * createmultipointobjecttool.h
 * Copyright 2014, Martin Ziel <martin.ziel.com>
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

#ifndef CREATEMULTIPOINTBJECTTOOL_H
#define CREATEMULTIPOINTBJECTTOOL_H

#include "createobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateMultipointObjectTool : public CreateObjectTool
{
    Q_OBJECT
public:
    CreateMultipointObjectTool(QObject* parent);
    void startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup);
    void languageChanged() = 0;

protected:
    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers, const bool snapToGrid, const bool snapToFineGrid);
    void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event, const bool snapToGrid, const bool snapToFineGrid);
};

}
}

#endif // CREATEMULTIPOINTBJECTTOOL_H
