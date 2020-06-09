/*
 * commandsedit.h
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

#include <QWidget>

namespace Ui {
class CommandsEdit;
}

namespace Tiled {

class CommandDataModel;

class CommandsEdit : public QWidget
{
    Q_OBJECT

public:
    explicit CommandsEdit(const QVector<Command> &commands, QWidget *parent = nullptr);
    ~CommandsEdit();

    const QVector<Command> &commands() const;

public slots:
    void setShortcut(const QKeySequence &keySequence);
    void setSaveBeforeExecute(int state);
    void setShowOutput(int state);
    void setExecutable(const QString &text);
    void setArguments(const QString &text);
    void setWorkingDirectory(const QString &text);

    void updateWidgets(const QModelIndex &current);

    void browseExecutable();
    void browseWorkingDirectory();

private:
    Ui::CommandsEdit *mUi;
    CommandDataModel *mModel;
};

} // namespace Tiled
