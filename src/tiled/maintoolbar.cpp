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
    QIcon openIcon(QLatin1String(":images/24/document-open.png"));
    QIcon saveIcon(QLatin1String(":images/24/document-save.png"));
    QIcon undoIcon(QLatin1String(":images/24/edit-undo.png"));
    QIcon redoIcon(QLatin1String(":images/24/edit-redo.png"));

    newIcon.addFile(QLatin1String(":images/16/document-new.png"));
    openIcon.addFile(QLatin1String(":images/16/document-open.png"));
    saveIcon.addFile(QLatin1String(":images/16/document-save.png"));
    redoIcon.addFile(QLatin1String(":images/16/edit-redo.png"));
    undoIcon.addFile(QLatin1String(":images/16/edit-undo.png"));

    mNewButton = new QToolButton(this);
    mOpenAction = new QAction(this);
    mSaveAction = new QAction(this);

    QMenu *newMenu = new QMenu(this);
    newMenu->addAction(ActionManager::action("NewMap"));
    newMenu->addAction(ActionManager::action("NewTileset"));
    mNewButton->setMenu(newMenu);
    mNewButton->setPopupMode(QToolButton::InstantPopup);

    QUndoGroup *undoGroup = DocumentManager::instance()->undoGroup();
    mUndoAction = undoGroup->createUndoAction(this, tr("Undo"));
    mRedoAction = undoGroup->createRedoAction(this, tr("Redo"));

    mNewButton->setIcon(newIcon);
    mOpenAction->setIcon(openIcon);
    mSaveAction->setIcon(saveIcon);
    mUndoAction->setIcon(undoIcon);
    mRedoAction->setIcon(redoIcon);

    Utils::setThemeIcon(mNewButton, "document-new");
    Utils::setThemeIcon(mOpenAction, "document-open");
    Utils::setThemeIcon(mSaveAction, "document-save");
    Utils::setThemeIcon(mRedoAction, "edit-redo");
    Utils::setThemeIcon(mUndoAction, "edit-undo");

    mRedoAction->setPriority(QAction::LowPriority);

    addWidget(mNewButton);
    addAction(mOpenAction);
    addAction(mSaveAction);
    addSeparator();
    addAction(mUndoAction);
    addAction(mRedoAction);
    addSeparator();
    addWidget(mCommandButton);

    DocumentManager *documentManager = DocumentManager::instance();
    connect(mOpenAction, &QAction::triggered, documentManager, &DocumentManager::openFileDialog);
    connect(mSaveAction, &QAction::triggered, documentManager, &DocumentManager::saveFile);

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
    mSaveAction->setEnabled(document);
    mCommandButton->setEnabled(document && !document->fileName().isEmpty());
}

void MainToolBar::retranslateUi()
{
    mNewButton->setToolTip(tr("New"));
    mOpenAction->setText(tr("Open"));
    mSaveAction->setText(tr("Save"));

    mRedoAction->setIconText(tr("Redo"));
    mUndoAction->setIconText(tr("Undo"));
}

} // namespace Tiled
