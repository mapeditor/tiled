/*
 * tilesetview.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilesetview.h"

#include "map.h"
#include "mapdocument.h"
#include "preferences.h"
#include "propertiesdialog.h"
#include "tmxmapwriter.h"
#include "tile.h"
#include "tileset.h"
#include "tilesetmodel.h"
#include "utils.h"
#include "zoomable.h"

#include <QAbstractItemDelegate>
#include <QCoreApplication>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QUndoCommand>
#include <QWheelEvent>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

/**
 * The delegate for drawing tile items in the tileset view.
 */
class TileDelegate : public QAbstractItemDelegate
{
public:
    TileDelegate(TilesetView *tilesetView, QObject *parent = 0)
        : QAbstractItemDelegate(parent)
        , mTilesetView(tilesetView)
    { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

private:
    TilesetView *mTilesetView;
};

void TileDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    // Draw the tile image
    const QVariant display = index.model()->data(index, Qt::DisplayRole);
    const QPixmap tileImage = display.value<QPixmap>();
    const int extra = mTilesetView->drawGrid() ? 1 : 0;

    if (mTilesetView->zoomable()->smoothTransform())
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

    painter->drawPixmap(option.rect.adjusted(0, 0, -extra, -extra), tileImage);

    // Overlay with highlight color when selected
    if (option.state & QStyle::State_Selected) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.5);
        painter->fillRect(option.rect.adjusted(0, 0, -extra, -extra),
                          option.palette.highlight());
        painter->setOpacity(opacity);
    }
}

QSize TileDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                             const QModelIndex &index) const
{
    const TilesetModel *m = static_cast<const TilesetModel*>(index.model());
    const Tileset *tileset = m->tileset();
    const qreal zoom = mTilesetView->zoomable()->scale();
    const int extra = mTilesetView->drawGrid() ? 1 : 0;

    return QSize(tileset->tileWidth() * zoom + extra,
                 tileset->tileHeight() * zoom + extra);
}



} // anonymous namespace

TilesetView::TilesetView(MapDocument *mapDocument, Zoomable *zoomable, QWidget *parent)
    : QTableView(parent)
    , mZoomable(zoomable)
    , mMapDocument(mapDocument)
{
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setItemDelegate(new TileDelegate(this, this));
    setShowGrid(false);

    QHeaderView *header = horizontalHeader();
    header->hide();
    header->setResizeMode(QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(1);

    header = verticalHeader();
    header->hide();
    header->setResizeMode(QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(1);

    // Hardcode this view on 'left to right' since it doesn't work properly
    // for 'right to left' languages.
    setLayoutDirection(Qt::LeftToRight);
    
    Preferences *prefs = Preferences::instance();
    mDrawGrid = prefs->showTilesetGrid();

    connect(mZoomable, SIGNAL(scaleChanged(qreal)), SLOT(adjustScale()));
    connect(prefs, SIGNAL(showTilesetGridChanged(bool)),
            SLOT(setDrawGrid(bool)));
}

QSize TilesetView::sizeHint() const
{
    return QSize(130, 100);
}

/**
 * Override to support zooming in and out using the mouse wheel.
 */
void TilesetView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier
        && event->orientation() == Qt::Vertical)
    {
        mZoomable->handleWheelDelta(event->delta());
        return;
    }

    QTableView::wheelEvent(event);
}

/**
 * Allow changing tile properties through a context menu.
 */
void TilesetView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    const TilesetModel *m = tilesetModel();
    Tile *tile = m->tileAt(index);

    const bool isExternal = m->tileset()->isExternal();
    QMenu menu;

    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));

    if (tile) {
        // Select this tile to make sure it is clear that only the properties
        // of a single tile are being edited.
        selectionModel()->setCurrentIndex(index,
                                          QItemSelectionModel::SelectCurrent |
                                          QItemSelectionModel::Clear);

        QAction *tileProperties = menu.addAction(propIcon,
                                                 tr("Tile &Properties..."));
        tileProperties->setEnabled(!isExternal);
        Utils::setThemeIcon(tileProperties, "document-properties");
        menu.addSeparator();

        connect(tileProperties, SIGNAL(triggered()),
                SLOT(editTileProperties()));
    }


    menu.addSeparator();
    QAction *toggleGrid = menu.addAction(tr("Show &Grid"));
    toggleGrid->setCheckable(true);
    toggleGrid->setChecked(mDrawGrid);

    Preferences *prefs = Preferences::instance();
    connect(toggleGrid, SIGNAL(toggled(bool)),
            prefs, SLOT(setShowTilesetGrid(bool)));

    menu.exec(event->globalPos());
}

void TilesetView::editTileProperties()
{
    const TilesetModel *m = tilesetModel();
    Tile *tile = m->tileAt(selectionModel()->currentIndex());
    if (!tile)
        return;

    PropertiesDialog propertiesDialog(tr("Tile"),
                                      tile,
                                      mMapDocument->undoStack(),
                                      this);
    propertiesDialog.exec();
}

void TilesetView::setDrawGrid(bool drawGrid)
{
    mDrawGrid = drawGrid;
    tilesetModel()->tilesetChanged();
}

void TilesetView::adjustScale()
{
    tilesetModel()->tilesetChanged();
}
