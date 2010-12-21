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

#include <QShortcut>

using namespace Tiled;
using namespace Tiled::Internal;

CommandDialog::CommandDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CommandDialog)
{
    mUi->setupUi(this);

    setWindowTitle(tr("Edit Commands"));

    mUi->treeView->setModel(&mModel);
    mUi->treeView->setColumnWidth(0, 200);
    mUi->treeView->setRootIsDecorated(false);

    QHeaderView *header = mUi->treeView->header();
    header->setStretchLastSection(false);
    header->setResizeMode(CommandDataModel::NameColumn,
                          QHeaderView::Interactive);
    header->setResizeMode(CommandDataModel::CommandColumn,
                          QHeaderView::Stretch);
    header->setResizeMode(CommandDataModel::EnabledColumn,
                          QHeaderView::ResizeToContents);

    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete,
                                              mUi->treeView);
    deleteShortcut->setContext(Qt::WidgetShortcut);
    connect(deleteShortcut, SIGNAL(activated()),
                            SLOT(deleteSelectedCommands()));

    mUi->saveBox->setChecked(mModel.saveBeforeExecute());
}

CommandDialog::~CommandDialog()
{
    delete mUi;
}

void CommandDialog::accept()
{
    QDialog::accept();

    mModel.setSaveBeforeExecute(mUi->saveBox->isChecked());
    mModel.commit();
}

void CommandDialog::deleteSelectedCommands()
{
    QItemSelectionModel *selection = mUi->treeView->selectionModel();
    const QModelIndexList indices = selection->selectedRows();
    if (!indices.isEmpty()) {
        mModel.deleteCommands(indices);
        selection->select(mUi->treeView->currentIndex(),
                          QItemSelectionModel::ClearAndSelect |
                          QItemSelectionModel::Rows);
    }
}
