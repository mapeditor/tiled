/*
 * tilestampdock.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

class QSortFilterProxyModel;

namespace Tiled {

class TileLayer;

class FilterEdit;
class TileStamp;
class TileStampManager;
class TileStampModel;
class TileStampView;

class TileStampsDock : public QDockWidget
{
    Q_OBJECT

public:
    TileStampsDock(TileStampManager *stampManager, QWidget *parent = nullptr);

signals:
    void setStamp(const TileStamp &);

protected:
    void changeEvent(QEvent *e) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    void indexPressed(const QModelIndex &index);
    void currentRowChanged(const QModelIndex &index);
    void showContextMenu(QPoint pos);

    void newStamp();
    void delete_();
    void duplicate();
    void addVariation();
    void chooseFolder();

    void ensureStampVisible(const TileStamp &stamp);

    void retranslateUi();
    void setStampAtIndex(const QModelIndex &index);

    TileStampManager *mTileStampManager;
    TileStampModel *mTileStampModel;
    QSortFilterProxyModel *mProxyModel;
    TileStampView *mTileStampView;
    FilterEdit *mFilterEdit;

    QAction *mNewStamp;
    QAction *mAddVariation;
    QAction *mDuplicate;
    QAction *mDelete;
    QAction *mChooseFolder;
};


/**
 * This view makes sure the size hint makes sense and implements the context
 * menu.
 */
class TileStampView : public QTreeView
{
    Q_OBJECT

public:
    explicit TileStampView(QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    bool event(QEvent *event) override;
};

} // namespace Tiled
