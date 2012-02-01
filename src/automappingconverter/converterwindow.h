/*
 * converterwindow.h
 * Copyright 2011, 2012, Stefan Beller, stefanbeller@googlemail.com
 *
 * This file is part of the AutomappingConverter, which converts old rulemaps
 * of Tiled to work with the latest version of Tiled.
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

#ifndef CONVERTERWINDOW_H
#define CONVERTERWINDOW_H

#include "converterdatamodel.h"
#include "convertercontrol.h"

#include <QMainWindow>
#include <QString>
#include <QList>

namespace Ui {
class MainWindow;
}

class ConverterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConverterWindow(QWidget *parent = 0);
    ~ConverterWindow();

public slots:
    void addRule();

private:
    Ui::MainWindow *ui;
    QString getVersion(QString filename);

    ConverterDataModel *mDataModel;
    ConverterControl *mControl;
};

#endif // CONVERTERWINDOW_H
