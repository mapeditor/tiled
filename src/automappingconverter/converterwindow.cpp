/*
 * converterwindow.cpp
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

#include "converterwindow.h"
#include "ui_converterwindow.h"

#include <QFileDialog>
#include <QDebug>

ConverterWindow::ConverterWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    mControl = new ConverterControl;
    mDataModel = new ConverterDataModel(mControl, this);

    ui->setupUi(this);
    ui->saveButton->setText(tr("Save all as %1").arg(mControl->version2()));

    connect(ui->addbutton, SIGNAL(clicked()), this, SLOT(addRule()));
    connect(ui->saveButton, SIGNAL(clicked()),
            mDataModel, SLOT(updateVersions()));

    ui->treeView->setModel(mDataModel);
    ui->treeView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
}

ConverterWindow::~ConverterWindow()
{
    delete ui;
    delete mControl;
}

void ConverterWindow::addRule()
{
    QString filter = tr("All Files (*)");
    filter += QLatin1String(";;");

    QString selectedFilter = tr("Tiled map files (*.tmx)");
    filter += selectedFilter;

    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Map"),
                                                          QString(), filter,
                                                          &selectedFilter);
    if (fileNames.isEmpty())
        return;

    mDataModel->insertFileNames(fileNames);
    ui->saveButton->setEnabled(true);
}
