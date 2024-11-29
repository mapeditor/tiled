/*
 * wangdock.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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
#include <QMap>

#include "wangset.h"

class QMenu;
class QModelIndex;
class QPushButton;
class QSortFilterProxyModel;
class QStackedWidget;
class QTabWidget;
class QToolBar;
class QToolButton;
class QTreeView;

namespace Tiled {

class ChangeEvent;
class Document;
class HasChildrenFilterModel;
class WangSetView;
class WangSetModel;
class WangColorModel;
class WangColorView;
class WangTemplateView;
class WangTemplateModel;
class TilesetDocument;
class TilesetDocumentsFilterModel;

class WangDock : public QDockWidget
{
    Q_OBJECT

public:
    WangDock(QWidget *parent = nullptr);
    ~WangDock() override;

    void setDocument(Document *document);

    WangSet *currentWangSet() const { return mCurrentWangSet; }
    WangId currentWangId() const { return mCurrentWangId; }
    int currentWangColor() const;

    void editWangSetName(WangSet *wangSet);
    void editWangColorName(int colorIndex);

    void setColorView();
    void hideTemplateColorView();

    WangColorView *wangColorView() const { return mWangColorView; }
    WangColorModel *wangColorModel() const { return mWangColorModel; }

signals:
    void currentWangSetChanged(WangSet *wangSet);
    void currentWangIdChanged(WangId wangId);

    void addWangSetRequested(WangSet::Type type);
    void duplicateWangSetRequested();
    void removeWangSetRequested();

    void selectWangBrush();
    // When the color view selection changes.
    void wangColorChanged(int color);

public slots:
    void setCurrentWangSet(WangSet *wangSet);
    void onCurrentWangIdChanged(WangId wangId);
    void onWangIdUsedChanged(WangId wangId);
    void setCurrentWangColor(int color);

protected:
    void changeEvent(QEvent *event) override;

private:
    void activateErase();
    void refreshCurrentWangSet();
    void refreshCurrentWangId();
    void refreshCurrentWangColor();
    void wangColorIndexPressed(const QModelIndex &index);
    void documentChanged(const ChangeEvent &change);
    void wangSetChanged(WangSet *wangSet);
    void wangSetIndexPressed(const QModelIndex &index);
    void expandRows(const QModelIndex &parent, int first, int last);
    void checkAnyWangSets();
    void addColor();
    void removeColor();

    void updateAddColorStatus();
    void retranslateUi();

    QModelIndex wangSetIndex(WangSet *wangSet) const;

    QToolBar *mWangSetToolBar;
    QToolBar *mWangColorToolBar;
    QToolButton *mNewWangSetButton;
    QMenu *mNewWangSetMenu;
    QAction *mAddCornerWangSet;
    QAction *mAddEdgeWangSet;
    QAction *mAddMixedWangSet;
    QAction *mDuplicateWangSet;
    QAction *mRemoveWangSet;
    QAction *mAddColor;
    QAction *mRemoveColor;

    Document *mDocument = nullptr;
    QStackedWidget *mStack;
    WangSetView *mWangSetView;
    QPushButton *mEraseWangIdsButton;
    WangSet *mCurrentWangSet = nullptr;
    WangId mCurrentWangId;
    TilesetDocumentsFilterModel *mTilesetDocumentFilterModel;
    WangColorView *mWangColorView;
    WangColorModel *mWangColorModel;
    QSortFilterProxyModel *mWangColorFilterModel;
    WangSetModel *mWangSetModel;
    HasChildrenFilterModel *mWangSetProxyModel;
    QWidget *mWangColorWidget;
    WangTemplateView *mWangTemplateView;
    WangTemplateModel *mWangTemplateModel;
    QTabWidget *mTemplateAndColorView;
    QWidget *mTemplateAndColorWidget;

    bool mInitializing = false;
};

} // namespace Tiled
