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
#include "commandmanager.h"
#include "utils.h"

#include <QShortcut>
#include <QMenu>
#include <QContextMenuEvent>
#include <QModelIndex>
#include <QFileDialog>
#include <QStandardPaths>

using namespace Tiled;
using namespace Tiled::Internal;

CommandDialog::CommandDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CommandDialog)
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle(tr("Edit Commands"));
    Utils::restoreGeometry(this);

    connect(mUi->saveBox, &QCheckBox::stateChanged,
            this, &CommandDialog::setSaveBeforeExecute);

    connect(mUi->outputBox, &QCheckBox::stateChanged,
            this, &CommandDialog::setShowOutput);

    connect(mUi->keySequenceEdit, &QKeySequenceEdit::keySequenceChanged,
            this, &CommandDialog::setShortcut);

    connect(mUi->executableEdit, &QLineEdit::textChanged,
            this, &CommandDialog::setExecutable);

    connect(mUi->argumentsEdit, &QLineEdit::textChanged,
            this, &CommandDialog::setArguments);

    connect(mUi->workingDirectoryEdit, &QLineEdit::textChanged,
            this, &CommandDialog::setWorkingDirectory);

    connect(mUi->treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &CommandDialog::updateWidgets);

    connect(mUi->exBrowseButton, &QPushButton::clicked,
            this, &CommandDialog::browseExecutable);

    connect(mUi->wdBrowseButton, &QPushButton::clicked,
            this, &CommandDialog::browseWorkingDirectory);
}

CommandDialog::~CommandDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void CommandDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);

    mUi->treeView->model()->commit();

    CommandManager::instance()->updateActions();
}

void CommandDialog::setShortcut(const QKeySequence &keySequence)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mUi->treeView->model()->rowCount())
        mUi->treeView->model()->setShortcut(current, keySequence);
}

void CommandDialog::setSaveBeforeExecute(int state)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mUi->treeView->model()->rowCount())
        mUi->treeView->model()->setSaveBeforeExecute(current, state);
}

void CommandDialog::setShowOutput(int state)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mUi->treeView->model()->rowCount())
        mUi->treeView->model()->setShowOutput(current, state);
}


void CommandDialog::setExecutable(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mUi->treeView->model()->rowCount())
        mUi->treeView->model()->setExecutable(current, text);
}

void CommandDialog::setArguments(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mUi->treeView->model()->rowCount())
        mUi->treeView->model()->setArguments(current, text);
}

void CommandDialog::setWorkingDirectory(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mUi->treeView->model()->rowCount())
        mUi->treeView->model()->setWorkingDirectory(current, text);
}

void CommandDialog::updateWidgets(const QModelIndex &current, const QModelIndex &)
{
    bool enable = (current.row() < mUi->treeView->model()->rowCount() - 1);

    mUi->saveBox->setEnabled(enable);
    mUi->executableEdit->setEnabled(enable);
    mUi->argumentsEdit->setEnabled(enable);
    mUi->workingDirectoryEdit->setEnabled(enable);
    mUi->exBrowseButton->setEnabled(enable);
    mUi->wdBrowseButton->setEnabled(enable);
    mUi->keySequenceEdit->setEnabled(enable);
    mUi->clearButton->setEnabled(enable);
    mUi->outputBox->setEnabled(enable);

    if (enable) {
        const Command command = mUi->treeView->model()->command(current);
        mUi->executableEdit->setText(command.executable);
        mUi->argumentsEdit->setText(command.arguments);
        mUi->workingDirectoryEdit->setText(command.workingDirectory);
        mUi->keySequenceEdit->setKeySequence(command.shortcut);
        mUi->saveBox->setChecked(command.saveBeforeExecute);
        mUi->outputBox->setChecked(command.showOutput);
    } else {
        mUi->executableEdit->clear();
        mUi->argumentsEdit->clear();
        mUi->workingDirectoryEdit->clear();
        mUi->keySequenceEdit->clear();
    }
}

void CommandDialog::browseExecutable()
{
    QString caption = tr("Select Executable");
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString executableName = QFileDialog::getOpenFileName(this, caption, dir);

    if (!executableName.isEmpty())
        mUi->executableEdit->setText(executableName);
}

void CommandDialog::browseWorkingDirectory()
{
    QString caption = tr("Select Working Directory");
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString workingDirectoryName = QFileDialog::getExistingDirectory(this, caption, dir,
                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!workingDirectoryName.isEmpty())
        mUi->workingDirectoryEdit->setText(workingDirectoryName);
}

CommandTreeView::CommandTreeView(QWidget *parent)
    : QTreeView(parent)
    , mModel(CommandManager::instance()->commandDataModel())
{
    setModel(mModel);
    setRootIsDecorated(false);

    // Setup resizing so the command column stretches
    setColumnWidth(0, 200);
    QHeaderView *h = header();
    h->setStretchLastSection(false);
    h->setSectionResizeMode(CommandDataModel::NameColumn, QHeaderView::Stretch);
    h->setSectionResizeMode(CommandDataModel::ShortcutColumn, QHeaderView::Fixed);
    h->setSectionResizeMode(CommandDataModel::EnabledColumn,
                            QHeaderView::ResizeToContents);

    // Allow deletion via keyboard
    QShortcut *d = new QShortcut(QKeySequence::Delete, this);
    d->setContext(Qt::WidgetShortcut);
    connect(d, SIGNAL(activated()), SLOT(removeSelectedCommands()));

    connect(mModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
                    SLOT(handleRowsRemoved(QModelIndex, int, int)));
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
