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

#include "documentmanager.h"
#include "utils.h"

#include <QAction>
#include <QEvent>
#include <QUndoGroup>

namespace Tiled {
namespace Internal {

MainToolBar::MainToolBar(QWidget *parent)
    : QToolBar(parent)
{
    setObjectName(QLatin1String("MainToolBar"));
    setWindowTitle(tr("Main Toolbar"));
    setToolButtonStyle(Qt::ToolButtonFollowStyle);

    QIcon openIcon(QLatin1String(":images/24x24/document-open.png"));
    QIcon saveIcon(QLatin1String(":images/24x24/document-save.png"));
    QIcon undoIcon(QLatin1String(":images/24x24/edit-undo.png"));
    QIcon redoIcon(QLatin1String(":images/24x24/edit-redo.png"));

    openIcon.addFile(QLatin1String(":images/16x16/document-open.png"));
    saveIcon.addFile(QLatin1String(":images/16x16/document-save.png"));
    redoIcon.addFile(QLatin1String(":images/16x16/edit-redo.png"));
    undoIcon.addFile(QLatin1String(":images/16x16/edit-undo.png"));

    mOpenAction = new QAction(this);
    mSaveAction = new QAction(this);

    QUndoGroup *undoGroup = DocumentManager::instance()->undoGroup();
    mUndoAction = undoGroup->createUndoAction(this, tr("Undo"));
    mRedoAction = undoGroup->createRedoAction(this, tr("Redo"));

    // todo: having a 'New' action on the tool bar requires having a dialog to
    // choose between making a new map or a new tileset. Or maybe just making
    // it only work for maps.
//    mUi->actionNew->setPriority(QAction::LowPriority);

    mOpenAction->setIcon(openIcon);
    mSaveAction->setIcon(saveIcon);
    mUndoAction->setIcon(undoIcon);
    mRedoAction->setIcon(redoIcon);

    Utils::setThemeIcon(mOpenAction, "document-open");
    Utils::setThemeIcon(mSaveAction, "document-save");
    Utils::setThemeIcon(mRedoAction, "edit-redo");
    Utils::setThemeIcon(mUndoAction, "edit-undo");

#if QT_VERSION == 0x050500
    mUndoAction->setPriority(QAction::LowPriority);
#endif
    mRedoAction->setPriority(QAction::LowPriority);

    addAction(mOpenAction);
    addAction(mSaveAction);
    addSeparator();
    addAction(mUndoAction);
    addAction(mRedoAction);

    DocumentManager *documentManager = DocumentManager::instance();
    connect(mOpenAction, SIGNAL(triggered(bool)), documentManager, SLOT(openFile()));
    connect(mSaveAction, SIGNAL(triggered(bool)), documentManager, SLOT(saveFile()));

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
}

void MainToolBar::retranslateUi()
{
    mOpenAction->setText(tr("Open"));
    mSaveAction->setText(tr("Save"));

    mRedoAction->setIconText(tr("Redo"));
    mUndoAction->setIconText(tr("Undo"));
}

} // namespace Internal
} // namespace Tiled
