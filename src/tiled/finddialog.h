/*
 * %finddialog.cpp%
 * Copyright 2017, Your Name <leon.moctezuma@gmail.com>
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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QApplication>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

#include "highlighttile.h"

namespace Tiled {

namespace Internal {

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    FindDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    static FindDialog *instance() { return mInstance; }
    static FindDialog *showDialog();

signals:
    void changeCoordinates(int x, int y);

private:
    void goToCoordinates();
    QLineEdit *lineEditX;
    QLineEdit *lineEditY;

    HighlightTile *highlightTile() const { return mHighlightTile; }
    HighlightTile *mHighlightTile;

    static FindDialog *mInstance;
};

}

}

#endif // FINDDIALOG_H
