/*
 * tilestampdock.cpp
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

#include "tilestampsdock.h"

#include "documentmanager.h"
#include "quickstampmanager.h"
#include "tilestamp.h"
#include "tilestampmodel.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QVBoxLayout>

namespace Tiled {
namespace Internal {

TileStampsDock::TileStampsDock(QuickStampManager *stampManager, QWidget *parent)
    : QDockWidget(parent)
    , mTileStampModel(stampManager->tileStampModel())
{
    setObjectName(QLatin1String("TileStampsDock"));
    retranslateUi();

    TileStampsView *stampsView = new TileStampsView(this);
    stampsView->setModel(mTileStampModel);
    stampsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    stampsView->header()->setStretchLastSection(false);
    stampsView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    stampsView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);
    layout->addWidget(stampsView);

    QItemSelectionModel *selectionModel = stampsView->selectionModel();
    connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentRowChanged(QModelIndex)));

    setWidget(widget);
}

void TileStampsDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void TileStampsDock::currentRowChanged(const QModelIndex &index)
{
    MapDocument *mapDocument = DocumentManager::instance()->currentDocument();
    if (!mapDocument)
        return;

    if (mTileStampModel->isStamp(index)) {
        emit setStamp(mTileStampModel->stampAt(index));
    } else if (const TileStampVariation *variation = mTileStampModel->variationAt(index)) {
        // single variation clicked, use it specifically
        emit setStamp(TileStamp(new Map(*variation->map)));
    }
}

void TileStampsDock::retranslateUi()
{
    setWindowTitle(tr("Tile Stamps"));
}


TileStampsView::TileStampsView(QWidget *parent)
    : QTreeView(parent)
{
}

QSize TileStampsView::sizeHint() const
{
    return QSize(130, 100);
}

void TileStampsView::contextMenuEvent(QContextMenuEvent *)
{
    // todo
}

void TileStampsView::keyPressEvent(QKeyEvent *event)
{
    MapDocument *mapDocument = DocumentManager::instance()->currentDocument();
    if (!mapDocument)
        return;

    const QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    TileStampModel *tileStampModel = static_cast<TileStampModel *>(model());

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        tileStampModel->removeRow(index.row(), index.parent());
        return;
    }

    QTreeView::keyPressEvent(event);
}

} // namespace Internal
} // namespace Tiled
