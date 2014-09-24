/*
 * createtileobjecttool.h
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

#ifndef CREATETILEOBJECTTOOL_H
#define CREATETILEOBJECTTOOL_H

#include "createobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateTileObjectTool : public CreateObjectTool
{
    Q_OBJECT
public:
    CreateTileObjectTool(QObject* parent);
    void languageChanged();

protected:
    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers, bool snapToGrid, bool snapToFineGrid);
    void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event, bool snapToGrid, bool snapToFineGrid);
    void mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event, bool snapToGrid, bool snapToFineGrid);
    MapObject* createNewMapObject();
};

}
}

#endif // CREATETILEOBJECTTOOL_H
