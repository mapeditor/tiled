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

class QModelIndex;
class QPushButton;
class QToolBar;

namespace Tiled {

class WangSet;

namespace Internal {

class Document;
class WangSetFilterModel;
class WangSetView;
class WangSetModel;
class TilesetDocument;

class WangDock : public QDockWidget
{
    Q_OBJECT

public:
    WangDock(QWidget *parent = nullptr);

    ~WangDock();

    void setDocument(Document *document);

    WangSet *currentWangSet() const { return mCurrentWangSet; }

    void editWangSetName(WangSet *wangSet);

signals:
    void currentWangSetChanged(const WangSet *WangSet);

    void addWangSetRequested();
    void removeWangSetRequested();

public slots:
    void setCurrentWangSet(WangSet *wangSet);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void refreshCurrentWangSet();
    void indexPressed(const QModelIndex &index);
    void expandRows(const QModelIndex &parent, int first, int last);

private:
    void retranslateUi();

    QModelIndex wangSetIndex(WangSet *wangSet) const;

    QToolBar *mToolBar;
    QAction *mAddWangSet;
    QAction *mRemoveWangSet;

    Document *mDocument;
    WangSetView *mWangSetView;
    WangSet *mCurrentWangSet;
    WangSetFilterModel *mProxyModel;

    bool mInitializing;
};

} // namespace Internal
} // namespace Tiled
