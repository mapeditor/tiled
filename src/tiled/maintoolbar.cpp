/*
 * maintoolbar.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "maintoolbar.h"

#include "actionmanager.h"
#include "commandbutton.h"
#include "documentmanager.h"
#include "utils.h"

#include <QAction>
#include <QEvent>
#include <QMenu>
#include <QToolButton>
#include <QUndoGroup>

namespace Tiled {

MainToolBar::MainToolBar(QWidget *parent)
    : QToolBar(parent)
    , mCommandButton(new CommandButton(this))
{
    setObjectName(QLatin1String("MainToolBar"));
    setWindowTitle(tr("Main Toolbar"));
    setToolButtonStyle(Qt::ToolButtonFollowStyle);

    QIcon newIcon(QLatin1String(":images/24/document-new.png"));
    newIcon.addFile(QLatin1String(":images/16/document-new.png"));

    mNewButton = new QToolButton(this);

    QMenu *newMenu = new QMenu(this);
    newMenu->addAction(ActionManager::action("NewMap"));
    newMenu->addAction(ActionManager::action("NewTileset"));
    mNewButton->setMenu(newMenu);
    mNewButton->setPopupMode(QToolButton::InstantPopup);
    mNewButton->setIcon(newIcon);

    Utils::setThemeIcon(mNewButton, "document-new");

    addWidget(mNewButton);
    addAction(ActionManager::action("Open"));
    addAction(ActionManager::action("Save"));
    addSeparator();
    addAction(ActionManager::action("Undo"));
    addAction(ActionManager::action("Redo"));
    addSeparator();
    addWidget(mCommandButton);

    DocumentManager *documentManager = DocumentManager::instance();

    connect(documentManager, &DocumentManager::currentDocumentChanged,
            this, &MainToolBar::currentDocumentChanged);

    connect(this, &MainToolBar::orientationChanged,
            this, &MainToolBar::onOrientationChanged);

    retranslateUi();
}

void MainToolBar::changeEvent(QEvent *event)
{
    QToolBar::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void MainToolBar::onOrientationChanged(Qt::Orientation orientation)
{
    setToolButtonStyle(orientation == Qt::Horizontal ? Qt::ToolButtonFollowStyle :
                                                       Qt::ToolButtonIconOnly);
}

void MainToolBar::currentDocumentChanged(Document *document)
{
    mCommandButton->setEnabled(document && !document->fileName().isEmpty());
}

void MainToolBar::retranslateUi()
{
    mNewButton->setToolTip(tr("New"));
}

} // namespace Tiled
