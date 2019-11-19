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
#include "issuescounter.h"
#include "newsbutton.h"
#include "newversionbutton.h"

#include <QAction>
#include <QStatusBar>

namespace Tiled {

NoEditorWidget::NoEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NoEditorWidget)
{
    ui->setupUi(this);

    // Transfer margin and spacing to the internal layout
    ui->verticalLayout->setMargin(ui->gridLayout->margin());
    ui->verticalLayout->setSpacing(ui->gridLayout->spacing());
    ui->gridLayout->setMargin(0);
    ui->gridLayout->setSpacing(0);

    // Add a status bar to the bottom
    auto statusBar = new QStatusBar;
    statusBar->addPermanentWidget(new NewsButton);
    statusBar->addPermanentWidget(new NewVersionButton(NewVersionButton::AutoVisible));
    statusBar->addWidget(new IssuesCounter);

    ui->gridLayout->addWidget(statusBar, 3, 0, 1, 3);

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
    ActionManager::action("NewMap")->trigger();
}

void NoEditorWidget::newTileset()
{
    ActionManager::action("NewTileset")->trigger();
}

void NoEditorWidget::openFile()
{
    DocumentManager::instance()->openFileDialog();
}

} // namespace Tiled
