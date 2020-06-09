/*
 * commanddialog.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2017, Ketan Gupta <ketan19972010@gmail.com>
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include "commandmanager.h"
#include "commandsedit.h"
#include "utils.h"

#include <QShortcut>
#include <QMenu>
#include <QContextMenuEvent>
#include <QModelIndex>

using namespace Tiled;

CommandDialog::CommandDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CommandDialog)
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    auto *commandManager = CommandManager::instance();

    mGlobalCommandsEdit = new CommandsEdit(commandManager->globalCommands());
    mProjectCommandsEdit = new CommandsEdit(commandManager->projectCommands());

    mUi->tabWidget->addTab(mGlobalCommandsEdit, tr("Global Commands"));
    mUi->tabWidget->addTab(mProjectCommandsEdit, tr("Project Commands"));

    Utils::restoreGeometry(this);
}

CommandDialog::~CommandDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

const QVector<Command> &CommandDialog::globalCommands() const
{
    return mGlobalCommandsEdit->commands();
}

const QVector<Command> &CommandDialog::projectCommands() const
{
    return mProjectCommandsEdit->commands();
}


CommandTreeView::CommandTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);

    // Allow deletion via keyboard
    QShortcut *d = new QShortcut(QKeySequence::Delete, this);
    d->setContext(Qt::WidgetShortcut);
    connect(d, &QShortcut::activated, this, &CommandTreeView::removeSelectedCommands);
}

void CommandTreeView::setModel(QAbstractItemModel *model)
{
    Q_ASSERT(qobject_cast<CommandDataModel*>(model) != nullptr);
    QTreeView::setModel(model);
}

/**
 * Returns the model used by this view casted to CommandDataModel.
 */
CommandDataModel *CommandTreeView::model() const
{
     return static_cast<CommandDataModel*>(QTreeView::model());
}

/**
 * Displays a context menu for the item at <i>event</i>'s position.
 */
void CommandTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());

    // Generate a run a menu for the index
    if (QMenu *menu = model()->contextMenu(this, index))
        menu->exec(event->globalPos());
}

/**
 * Brings the selection to safety before rows will get removed.
 */
void CommandTreeView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid())
        return;

    int selectedRow = currentIndex().row();
    if (selectedRow >= start && selectedRow <= end && end < model()->rowCount() - 1)
        setCurrentIndex(model()->index(end + 1, 0));

    QTreeView::rowsAboutToBeRemoved(parent, start, end);
}

/**
 * Gets the currently selected rows and tells the model to delete them.
 */
void CommandTreeView::removeSelectedCommands()
{
    const QModelIndexList indices = selectionModel()->selectedRows();
    model()->removeRows(indices);
}
