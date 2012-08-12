/*
 * mapsdock.h
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

#ifndef MAPSDOCK_H
#define MAPSDOCK_H

#include <QDockWidget>
#include <QTreeView>

class QFileSystemModel;
class QLabel;
class QLineEdit;
class QModelIndex;
class QTreeView;

namespace Tiled {
namespace Internal {

class MainWindow;
class MapsView;

class MapsDock : public QDockWidget
{
    Q_OBJECT

public:
    MapsDock(MainWindow *mainWindow, QWidget *parent = 0);

private slots:
    void browse();
    void editedMapsDirectory();
    void onMapsDirectoryChanged();

protected:
    void changeEvent(QEvent *e);

private:
    void retranslateUi();

    QLineEdit *mDirectoryEdit;
    MapsView *mMapsView;
};

/**
 * Shows the list of files and directories.
 */
class MapsView : public QTreeView
{
    Q_OBJECT

public:
    MapsView(MainWindow *mainWindow, QWidget *parent = 0);

    /**
     * Returns a sensible size hint.
     */
    QSize sizeHint() const;

    void mousePressEvent(QMouseEvent *event);

    QFileSystemModel *model() const { return mFSModel; }

private slots:
    void onMapsDirectoryChanged();
    void onActivated(const QModelIndex &index);

private:
    MainWindow *mMainWindow;
    QFileSystemModel *mFSModel;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPSDOCK_H
