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

CommandDialog::CommandDialog(const QVector<Command> &commands, QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CommandDialog)
    , mModel(new CommandDataModel(this))
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    mModel->setCommands(commands);
    mUi->treeView->setModel(mModel);

    // Setup resizing so the command column stretches
    QHeaderView *h = mUi->treeView->header();
    h->resizeSection(0, Utils::dpiScaled(200));
    h->setStretchLastSection(false);
    h->setSectionResizeMode(CommandDataModel::NameColumn, QHeaderView::Stretch);
    h->setSectionResizeMode(CommandDataModel::ShortcutColumn, QHeaderView::Fixed);
    h->setSectionResizeMode(CommandDataModel::EnabledColumn, QHeaderView::ResizeToContents);

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

const QVector<Command> &CommandDialog::commands() const
{
    return mModel->commands();
}

void CommandDialog::setShortcut(const QKeySequence &keySequence)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setShortcut(current, keySequence);
}

void CommandDialog::setSaveBeforeExecute(int state)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setSaveBeforeExecute(current, state);
}

void CommandDialog::setShowOutput(int state)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setShowOutput(current, state);
}


void CommandDialog::setExecutable(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setExecutable(current, text);
}

void CommandDialog::setArguments(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setArguments(current, text);
}

void CommandDialog::setWorkingDirectory(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setWorkingDirectory(current, text);
}

void CommandDialog::updateWidgets(const QModelIndex &current)
{
    bool enable = (current.row() < mModel->rowCount() - 1);

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
        const Command command = mModel->command(current);
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
    if (selectedRow >= start && selectedRow <= end && end < model()->rowCount() - 1) {
        selectionModel()->select(model()->index(end + 1, 0),
                                 QItemSelectionModel::ClearAndSelect |
                                 QItemSelectionModel::Rows);
    }

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
