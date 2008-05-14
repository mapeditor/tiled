/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "mainwindow.h"

#include "layer.h"
#include "map.h"
#include "mapscene.h"
#include "resizedialog.h"
#include "xmlmapreader.h"

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

using namespace Tiled::Internal;

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    mUi.setupUi(this);
    mUi.splitter->setCollapsible(0, false);

    connect(mUi.actionOpen, SIGNAL(triggered()), SLOT(openFile()));
    connect(mUi.actionSave, SIGNAL(triggered()), SLOT(saveFile()));
    connect(mUi.actionQuit, SIGNAL(triggered()), SLOT(close()));
    connect(mUi.actionResize, SIGNAL(triggered()), SLOT(resizeMap()));
    connect(mUi.actionAbout, SIGNAL(triggered()), SLOT(aboutTiled()));
    connect(mUi.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    mScene = new MapScene(this);
    mUi.graphicsView->setScene(mScene);
    mUi.graphicsView->centerOn(0, 0);
    mUi.actionResize->setEnabled(false);
}

MainWindow::~MainWindow()
{
}

void MainWindow::openFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this);

    // Use the XML map reader to read the map (assuming it's a .tmx file)
    if (!fileName.isEmpty()) {
        qDebug() << "Loading map:" << fileName;
        XmlMapReader mapReader;
        Map *map = mapReader.read(fileName);
        Map *previousMap = mScene->map();
        mScene->setMap(map);
        mUi.graphicsView->centerOn(0, 0);
        mUi.actionResize->setEnabled(map != 0);
        delete previousMap;
    }
}

void MainWindow::saveFile()
{
    const QString fileName = QFileDialog::getSaveFileName(this);
    qDebug() << fileName;
}

void MainWindow::resizeMap()
{
    ResizeDialog resizeDialog(this);
    resizeDialog.setOldSize(QSize(mScene->map()->width(),
                                  mScene->map()->height()));

    if (resizeDialog.exec()) {
        mScene->map()->setWidth(resizeDialog.newSize().width());
        mScene->map()->setHeight(resizeDialog.newSize().height());
    }
    // TODO: Actually implement map resizing
}

void MainWindow::aboutTiled()
{
    // TODO: Implement about dialog
}
