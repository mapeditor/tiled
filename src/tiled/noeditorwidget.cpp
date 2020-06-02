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
#include "mainwindow.h"
#include "stylehelper.h"
#include "tiledproxystyle.h"
#include "utils.h"

#include <QAction>
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QMenu>

namespace Tiled {

NoEditorWidget::NoEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NoEditorWidget)
{
    ui->setupUi(this);

    auto opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.25);
    ui->logo->setGraphicsEffect(opacityEffect);

    ui->versionLabel->setText(QString(QLatin1String("%1 %2")).arg(QGuiApplication::applicationDisplayName(), QGuiApplication::applicationVersion()));

    connect(ui->openProjectButton, &QToolButton::clicked, ActionManager::action("OpenProject"), &QAction::trigger);
    connect(ui->saveProjectButton, &QToolButton::clicked, ActionManager::action("SaveProjectAs"), &QAction::trigger);
    connect(ui->addFolderToProjectButton, &QToolButton::clicked, ActionManager::action("AddFolderToProject"), &QAction::trigger);

    connect(ui->newMapButton, &QToolButton::clicked, this, &NoEditorWidget::newMap);
    connect(ui->newTilesetButton, &QToolButton::clicked, this, &NoEditorWidget::newTileset);
    connect(ui->openFileButton, &QToolButton::clicked, this, &NoEditorWidget::openFile);

    Preferences *preferences = Preferences::instance();
    connect(preferences, &Preferences::recentProjectsChanged, this, &NoEditorWidget::updateRecentProjectsMenu);

    connect(StyleHelper::instance(), &StyleHelper::styleApplied, this, &NoEditorWidget::adjustToStyle);

    updateRecentProjectsMenu();
    adjustToStyle();
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
        retranslateUi();
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

void NoEditorWidget::retranslateUi()
{
    ui->retranslateUi(this);

    ui->openProjectButton->setText(ActionManager::action("OpenProject")->text());
    ui->saveProjectButton->setText(ActionManager::action("SaveProjectAs")->text());
    ui->addFolderToProjectButton->setText(ActionManager::action("AddFolderToProject")->text());
}

void NoEditorWidget::updateRecentProjectsMenu()
{
    auto menu = ui->openProjectButton->menu();
    if (!menu)
        menu = new QMenu(this);

    menu->clear();

    bool enabled = MainWindow::instance()->addRecentProjectsActions(menu);

    if (enabled) {
        ui->openProjectButton->setMenu(menu);
    } else {
        ui->openProjectButton->setMenu(nullptr);
        delete menu;
    }

    ui->openProjectButton->setPopupMode(enabled ? QToolButton::MenuButtonPopup
                                                : QToolButton::DelayedPopup);
}

void NoEditorWidget::adjustToStyle()
{
    if (auto *style = qobject_cast<TiledProxyStyle*>(QApplication::style())) {
        if (style->isDark())
            ui->logo->setPixmap(QPixmap(QString::fromUtf8(":/images/about-tiled-logo-white.png")));
        else
            ui->logo->setPixmap(QPixmap(QString::fromUtf8(":/images/about-tiled-logo.png")));
    }
}

} // namespace Tiled
