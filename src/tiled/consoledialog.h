/*
 * consoledialog.h
 * Copyright 2013, Samuli Tuomola <samuli.tuomola@gmail.com>
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

#ifndef CONSOLEDIALOG_H
#define CONSOLEDIALOG_H

#include <QDialog>
#include <QString>
#include "consoleinterface.h"

namespace Ui {
class ConsoleDialog;
}

class ConsoleDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConsoleDialog(QWidget *parent = 0);
    ~ConsoleDialog();

protected slots:
    void appendInfo(QString str);
    void appendError(QString str);

private:
    Ui::ConsoleDialog *ui;
    QColor infoColor;
    QColor errorColor;
};

#endif // CONSOLEDIALOG_H
