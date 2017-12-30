/*
 * noeditorwidget.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "noeditorwidget.h"
#include "ui_noeditorwidget.h"

#include "actionmanager.h"
#include "documentmanager.h"

namespace Tiled {
namespace Internal {

NoEditorWidget::NoEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NoEditorWidget)
{
    ui->setupUi(this);

    connect(ui->newMapButton, &QPushButton::clicked, this, &NoEditorWidget::newMap);
    connect(ui->newTilesetButton, &QPushButton::clicked, this, &NoEditorWidget::newTileset);
    connect(ui->openFileButton, &QPushButton::clicked, this, &NoEditorWidget::openFile);
}

NoEditorWidget::~NoEditorWidget()
{
    delete ui;
}

void NoEditorWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void NoEditorWidget::newMap()
{
    ActionManager::action("file.new_map")->trigger();
}

void NoEditorWidget::newTileset()
{
    ActionManager::action("file.new_tileset")->trigger();
}

void NoEditorWidget::openFile()
{
    DocumentManager::instance()->openFile();
}

} // namespace Internal
} // namespace Tiled
