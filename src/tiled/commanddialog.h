/*
 * commanddialog.h
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

#ifndef COMMANDDIALOG_H
#define COMMANDDIALOG_H

#include "commanddatamodel.h"

#include <QDialog>
#include <QSettings>

namespace Ui {
class CommandDialog;
}

namespace Tiled {
namespace Internal {

class CommandDialog : public QDialog
{
    Q_OBJECT

public:
    CommandDialog(QWidget *parent = 0);
    ~CommandDialog();

    /**
      * Saves the changes to the users preferences.
      * Automatically called when the dialog is accepted.
      */
    void accept();

private slots:

    /**
      * Gets the currently selected rows and tells the model to delete them.
      */
    void deleteSelectedCommands();

private:
    Ui::CommandDialog *mUi;
    CommandDataModel mModel;
};


} // namespace Internal
} // namespace Tiled

#endif // COMMANDDIALOG_H
