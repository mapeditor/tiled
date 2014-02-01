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

#ifndef OBJECTSDOCK_H
#define OBJECTSDOCK_H

#include <QDockWidget>
#include <QTreeView>

class QTreeView;

namespace Tiled {

class ObjectGroup;

namespace Internal {

class MapDocument;
class MapObjectModel;
class ObjectsView;

class ObjectsDock : public QDockWidget
{
    Q_OBJECT

public:
    ObjectsDock(QWidget *parent = 0);

    void setMapDocument(MapDocument *mapDoc);

protected:
    void changeEvent(QEvent *e);

private slots:
    void updateActions();
    void aboutToShowMoveToMenu();
    void triggeredMoveToMenu(QAction *action);
    void objectProperties();
    void documentCloseRequested(int index);

private:
    void retranslateUi();

    void saveExpandedGroups(MapDocument *mapDoc);
    void restoreExpandedGroups(MapDocument *mapDoc);

    QAction *mActionNewLayer;
    QAction *mActionObjectProperties;
    QAction *mActionMoveToGroup;

    ObjectsView *mObjectsView;
    MapDocument *mMapDocument;
    QMap<MapDocument*, QList<ObjectGroup*> > mExpandedGroups;
    QMenu *mMoveToMenu;
};

class ObjectsView : public QTreeView
{
    Q_OBJECT

public:
    ObjectsView(QWidget *parent = 0);

    QSize sizeHint() const;

    void setMapDocument(MapDocument *mapDoc);

    MapObjectModel *model() const;

protected slots:
    virtual void selectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected);

private slots:
    void onPressed(const QModelIndex &index);
    void onActivated(const QModelIndex &index);
    void selectedObjectsChanged();

private:
    MapDocument *mMapDocument;
    bool mSynching;
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTSDOCK_H
