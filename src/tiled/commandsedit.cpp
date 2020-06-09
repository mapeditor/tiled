/*
 * commandsedit.cpp
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

#include "commandsedit.h"
#include "ui_commandsedit.h"

#include "commanddatamodel.h"
#include "utils.h"

#include <QFileDialog>
#include <QStandardPaths>

namespace Tiled {

CommandsEdit::CommandsEdit(const QVector<Command> &commands, QWidget *parent)
    : QWidget(parent)
    , mUi(new Ui::CommandsEdit)
    , mModel(new CommandDataModel(this))
{
    mUi->setupUi(this);

    mModel->setCommands(commands);
    mUi->treeView->setModel(mModel);

    // Setup resizing so the command column stretches
    QHeaderView *h = mUi->treeView->header();
    h->resizeSection(0, Utils::dpiScaled(200));
    h->setStretchLastSection(false);
    h->setSectionResizeMode(CommandDataModel::NameColumn, QHeaderView::Stretch);
    h->setSectionResizeMode(CommandDataModel::ShortcutColumn, QHeaderView::Fixed);
    h->setSectionResizeMode(CommandDataModel::EnabledColumn, QHeaderView::ResizeToContents);

    connect(mUi->saveBox, &QCheckBox::stateChanged,
            this, &CommandsEdit::setSaveBeforeExecute);

    connect(mUi->outputBox, &QCheckBox::stateChanged,
            this, &CommandsEdit::setShowOutput);

    connect(mUi->keySequenceEdit, &QKeySequenceEdit::keySequenceChanged,
            this, &CommandsEdit::setShortcut);

    connect(mUi->executableEdit, &QLineEdit::textChanged,
            this, &CommandsEdit::setExecutable);

    connect(mUi->argumentsEdit, &QLineEdit::textChanged,
            this, &CommandsEdit::setArguments);

    connect(mUi->workingDirectoryEdit, &QLineEdit::textChanged,
            this, &CommandsEdit::setWorkingDirectory);

    connect(mUi->treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &CommandsEdit::updateWidgets);

    connect(mUi->exBrowseButton, &QPushButton::clicked,
            this, &CommandsEdit::browseExecutable);

    connect(mUi->wdBrowseButton, &QPushButton::clicked,
            this, &CommandsEdit::browseWorkingDirectory);
}

CommandsEdit::~CommandsEdit()
{
    delete mUi;
}

const QVector<Command> &CommandsEdit::commands() const
{
    return mModel->commands();
}

void CommandsEdit::setShortcut(const QKeySequence &keySequence)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setShortcut(current, keySequence);
}

void CommandsEdit::setSaveBeforeExecute(int state)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setSaveBeforeExecute(current, state);
}

void CommandsEdit::setShowOutput(int state)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setShowOutput(current, state);
}


void CommandsEdit::setExecutable(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setExecutable(current, text);
}

void CommandsEdit::setArguments(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setArguments(current, text);
}

void CommandsEdit::setWorkingDirectory(const QString &text)
{
    const QModelIndex &current = mUi->treeView->currentIndex();
    if (current.row() < mModel->rowCount())
        mModel->setWorkingDirectory(current, text);
}

void CommandsEdit::updateWidgets(const QModelIndex &current)
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

void CommandsEdit::browseExecutable()
{
    QString caption = tr("Select Executable");
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString executableName = QFileDialog::getOpenFileName(this, caption, dir);

    if (!executableName.isEmpty())
        mUi->executableEdit->setText(executableName);
}

void CommandsEdit::browseWorkingDirectory()
{
    QString caption = tr("Select Working Directory");
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString workingDirectoryName = QFileDialog::getExistingDirectory(this, caption, dir,
                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!workingDirectoryName.isEmpty())
        mUi->workingDirectoryEdit->setText(workingDirectoryName);
}

} // namespace Tiled
