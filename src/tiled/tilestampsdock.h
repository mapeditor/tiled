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

#ifndef TILED_INTERNAL_TILESTAMPSDOCK_H
#define TILED_INTERNAL_TILESTAMPSDOCK_H

#include <QDockWidget>
#include <QTreeView>

namespace Tiled {

class TileLayer;

namespace Internal {

class QuickStampManager;
class TileStamp;
class TileStampModel;

class TileStampsDock : public QDockWidget
{
    Q_OBJECT

public:
    TileStampsDock(QuickStampManager *stampManager, QWidget *parent = 0);

signals:
    void setStamp(const TileStamp &);

protected:
    void changeEvent(QEvent *e);

private slots:
    void currentRowChanged(const QModelIndex &index);

private:
    void retranslateUi();

    TileStampModel *mTileStampModel;
};


/**
 * This view makes sure the size hint makes sense and implements the context
 * menu.
 */
class TileStampsView : public QTreeView
{
    Q_OBJECT

public:
    explicit TileStampsView(QWidget *parent = 0);

    QSize sizeHint() const;

protected:
    void contextMenuEvent(QContextMenuEvent *);
    void keyPressEvent(QKeyEvent *);
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILESTAMPSDOCK_H
