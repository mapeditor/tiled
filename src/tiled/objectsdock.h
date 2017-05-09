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

#include <QDockWidget>
#include <QTreeView>
#include <QSortFilterProxyModel>

class QAbstractProxyModel;
class QTreeView;

namespace Tiled {

class ObjectGroup;

namespace Internal {

class Document;
class MapDocument;
class MapObjectModel;
class ObjectsView;

class ObjectsFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ObjectsFilter(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
private :
    bool pass(const QModelIndex index) const;
};

class ObjectsDock : public QDockWidget
{
    Q_OBJECT

public:
    ObjectsDock(QWidget *parent = nullptr);

    void setMapDocument(MapDocument *mapDoc);

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void updateActions();
    void aboutToShowMoveToMenu();
    void triggeredMoveToMenu(QAction *action);
    void objectProperties();
    void documentAboutToClose(Document *document);
    void moveObjectsUp();
    void moveObjectsDown();

private:
    void retranslateUi();

    void saveExpandedGroups();
    void restoreExpandedGroups();

    QAction *mActionNewLayer;
    QAction *mActionObjectProperties;
    QAction *mActionMoveToGroup;
    QAction *mActionMoveUp;
    QAction *mActionMoveDown;

    QLineEdit *mFilterEdit;
    ObjectsView *mObjectsView;
    MapDocument *mMapDocument;
    QMap<MapDocument*, QList<ObjectGroup*> > mExpandedGroups;
    QMenu *mMoveToMenu;
};

class ObjectsView : public QTreeView
{
    Q_OBJECT

public:
    ObjectsView(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void setMapDocument(MapDocument *mapDoc);

    MapObjectModel *mapObjectModel() const;
    QSortFilterProxyModel *mObjectsFilterModel;
    QAbstractProxyModel *mProxyModel;

protected:
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

private slots:
    void onPressed(const QModelIndex &proxyIndex);
    void onActivated(const QModelIndex &proxyIndex);
    void onSectionResized(int logicalIndex);
    void selectedObjectsChanged();
    void setColumnVisibility(bool visible);

    void showCustomMenu(const QPoint &point);

private:
    void restoreVisibleSections();
    void synchronizeSelectedItems();

    MapDocument *mMapDocument;
    bool mSynching;
};

} // namespace Internal
} // namespace Tiled
