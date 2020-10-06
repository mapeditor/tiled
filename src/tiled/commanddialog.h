/*
 * commanddialog.h
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

#pragma once

#include "command.h"

#include <QDialog>
#include <QTreeView>

namespace Ui {
class CommandDialog;
}

namespace Tiled {

class CommandDataModel;
class CommandsEdit;

class CommandDialog : public QDialog
{
    Q_OBJECT

public:
    CommandDialog(QWidget *parent = nullptr);
    ~CommandDialog();

    const QVector<Command> &globalCommands() const;
    const QVector<Command> &projectCommands() const;

private:
    Ui::CommandDialog *mUi;
    CommandsEdit *mGlobalCommandsEdit;
    CommandsEdit *mProjectCommandsEdit;
};

class CommandTreeView : public QTreeView
{
    Q_OBJECT

public:
    CommandTreeView(QWidget *parent);

    void setModel(QAbstractItemModel *model) override;
    CommandDataModel *model() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

private:
    void handleRowsRemoved(const QModelIndex &parent, int start, int end);

    void removeSelectedCommands();
};

} // namespace Tiled
