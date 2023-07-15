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

    ui->logo->setPixmap(QPixmap(QString::fromUtf8(":/images/about-tiled-logo.png")));

    auto opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.25);
    ui->logo->setGraphicsEffect(opacityEffect);

    ui->versionLabel->setText(QStringLiteral("%1 %2").arg(QGuiApplication::applicationDisplayName(), QGuiApplication::applicationVersion()));

    connect(ui->newProjectButton, &QToolButton::clicked, ActionManager::action("NewProject"), &QAction::trigger);

    connect(ui->newMapButton, &QToolButton::clicked, this, &NoEditorWidget::newMap);
    connect(ui->newTilesetButton, &QToolButton::clicked, this, &NoEditorWidget::newTileset);
    connect(ui->openFileButton, &QToolButton::clicked, this, &NoEditorWidget::openFile);

    Preferences *preferences = Preferences::instance();
    connect(preferences, &Preferences::recentProjectsChanged, this, &NoEditorWidget::updateRecentProjectsMenu);

    connect(StyleHelper::instance(), &StyleHelper::styleApplied, this, &NoEditorWidget::adjustToStyle);

    updateRecentProjectsMenu();
    adjustToStyle();
    retranslateUi();
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
    ui->newProjectButton->setText(ActionManager::action("NewProject")->text());
    ui->openFileButton->setText(ActionManager::action("Open")->text());
}

void NoEditorWidget::updateRecentProjectsMenu()
{
    auto menu = ui->recentProjectsButton->menu();
    if (!menu)
        menu = new QMenu(this);

    menu->clear();

    bool enabled = MainWindow::instance()->addRecentProjectsActions(menu);

    ui->recentProjectsButton->setMenu(menu);
    ui->recentProjectsButton->setEnabled(enabled);
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

#include "moc_noeditorwidget.cpp"
