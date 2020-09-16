/*
 * objectreferencetool.h
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "abstractobjecttool.h"
#include "mapdocument.h"

#include <QList>

namespace Tiled {

class ObjectReferenceItem;

class ObjectReferenceTool : public AbstractObjectTool
{
public:
    explicit ObjectReferenceTool(QObject *parent = nullptr);

    void activate(MapScene *) override;
    void deactivate(MapScene *) override;

    void keyPressed(QKeyEvent *) override;
    void mouseEntered() override;
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *) override;
    void mouseReleased(QGraphicsSceneMouseEvent *) override;

protected:
    void changeEvent(const ChangeEvent &) override;
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

private:
    void setItemsVisible(bool visible);
    void startPick();
    void endPick();
    void pickObject(MapObject *mapObject);
    void updateReferenceItems();

    QList<ObjectReferenceItem*> mReferenceItems;
    QPointF mTargetPos;
    bool mPicking = false;
    bool mItemsVisible = false;
    QPointer<AbstractTool> mPreviousTool;
};

} // namespace Tiled
