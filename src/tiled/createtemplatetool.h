/*
 * createtemplatetool.h
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#pragma once

#include "createobjecttool.h"

namespace Tiled {

class ObjectTemplate;

namespace Internal {

class CreateTemplateTool : public CreateObjectTool
{
    Q_OBJECT

public:
    CreateTemplateTool(QObject *parent);

    void languageChanged() override;

protected:
    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers) override;
    void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event) override;
    void mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event) override;

    bool startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup) override;
    MapObject *createNewMapObject() override;
};

} // namespace Internal
} // namespace Tiled
