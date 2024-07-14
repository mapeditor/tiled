/*
 * objectsdock.h
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
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

#include "editableobjectgroup.h"

#include <QDockWidget>
#include <QMap>

class QMenu;

namespace Tiled {

class Document;
class FilterEdit;
class MapDocument;
class ObjectsView;

class ObjectsDock : public QDockWidget
{
    Q_OBJECT

public:
    ObjectsDock(QWidget *parent = nullptr);

    void setMapDocument(MapDocument *mapDoc);

    Q_INVOKABLE bool isExpanded(EditableObjectGroup *layer) const { return isExpanded(layer->objectGroup()); };
    Q_INVOKABLE void setExpanded(EditableObjectGroup *layer, bool expanded) { setExpanded(layer->objectGroup(), expanded); };
    bool isExpanded(ObjectGroup *layer) const;
    void setExpanded(ObjectGroup *layer, bool expanded);

protected:
    void changeEvent(QEvent *e) override;

private:
    void updateActions();
    void aboutToShowMoveToMenu();
    void triggeredMoveToMenu(QAction *action);
    void objectProperties();
    void moveObjectsUp();
    void moveObjectsDown();

    void retranslateUi();

    QAction *mActionNewLayer;
    QAction *mActionObjectProperties;
    QAction *mActionMoveToGroup;
    QAction *mActionMoveUp;
    QAction *mActionMoveDown;

    FilterEdit *mFilterEdit;
    ObjectsView *mObjectsView;
    MapDocument *mMapDocument;
    QMenu *mMoveToMenu;
};

} // namespace Tiled
