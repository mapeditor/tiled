/*
 * commanddialog.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "commanddialog.h"
#include "ui_commanddialog.h"

#include "commanddatamodel.h"
#include "utils.h"

#include <QShortcut>
#include <QMenu>
#include <QContextMenuEvent>
#include <QModelIndex>

using namespace Tiled;
using namespace Tiled::Internal;

CommandDialog::CommandDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CommandDialog)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mUi->saveBox->setChecked(mUi->treeView->model()->saveBeforeExecute());

    setWindowTitle(tr("Edit Commands"));
    Utils::restoreGeometry(this);
}

CommandDialog::~CommandDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void CommandDialog::accept()
{
    QDialog::accept();

    mUi->treeView->model()->setSaveBeforeExecute(mUi->saveBox->isChecked());
    mUi->treeView->model()->commit();
}

CommandTreeView::CommandTreeView(QWidget *parent)
    : QTreeView(parent)
    , mModel(new CommandDataModel)
{
    setModel(mModel);
    setRootIsDecorated(false);

    // Setup resizing so the command column stretches
    setColumnWidth(0, 200);
    QHeaderView *h = header();
    h->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
    h->setSectionResizeMode(CommandDataModel::NameColumn, QHeaderView::Interactive);
    h->setSectionResizeMode(CommandDataModel::CommandColumn, QHeaderView::Stretch);
    h->setSectionResizeMode(CommandDataModel::EnabledColumn,
                            QHeaderView::ResizeToContents);
#else
    h->setResizeMode(CommandDataModel::NameColumn, QHeaderView::Interactive);
    h->setResizeMode(CommandDataModel::CommandColumn, QHeaderView::Stretch);
    h->setResizeMode(CommandDataModel::EnabledColumn,
                     QHeaderView::ResizeToContents);
#endif

    // Allow deletion via keyboard
    QShortcut *d = new QShortcut(QKeySequence::Delete, this);
    d->setContext(Qt::WidgetShortcut);
    connect(d, SIGNAL(activated()), SLOT(removeSelectedCommands()));

    connect(mModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
                    SLOT(handleRowsRemoved(QModelIndex, int, int)));
}

CommandTreeView::~CommandTreeView()
{
    delete mModel;
}

void CommandTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());

    // Generate a run a menu for the index
    QMenu *menu = mModel->contextMenu(this, index);
    if (menu)
        menu->exec(event->globalPos());
}

void CommandTreeView::handleRowsRemoved(const QModelIndex &parent, int, int)
{
    if (parent.isValid())
        return;

    // Reselect the same row index of the removed row
    QItemSelectionModel *sModel = selectionModel();
    QModelIndex index = sModel->currentIndex();

    sModel->select(index.sibling(index.row() + 1,index.column()),
                   QItemSelectionModel::ClearAndSelect |
                   QItemSelectionModel::Rows);
}

void CommandTreeView::removeSelectedCommands()
{
    QItemSelectionModel *selection = selectionModel();
    const QModelIndexList indices = selection->selectedRows();
    mModel->removeRows(indices);
}
