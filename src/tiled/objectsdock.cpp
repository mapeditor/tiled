/*
 * objectsdock.cpp
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
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

#include "objectsdock.h"

#include "actionmanager.h"
#include "documentmanager.h"
#include "filteredit.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "objectsview.h"
#include "raiselowerhelper.h"
#include "utils.h"

#include <QBoxLayout>
#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>

using namespace Tiled;

ObjectsDock::ObjectsDock(QWidget *parent)
    : QDockWidget(parent)
    , mFilterEdit(new FilterEdit(this))
    , mObjectsView(new ObjectsView)
    , mMapDocument(nullptr)
{
    setObjectName(QLatin1String("ObjectsDock"));

    mActionObjectProperties = new QAction(this);
    mActionObjectProperties->setIcon(QIcon(QLatin1String(":/images/16/document-properties.png")));

    connect(mActionObjectProperties, &QAction::triggered,
            this, &ObjectsDock::objectProperties);

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mFilterEdit);
    layout->addWidget(mObjectsView);

    mFilterEdit->setFilteredView(mObjectsView);

    connect(mFilterEdit, &QLineEdit::textChanged, mObjectsView, &ObjectsView::setFilter);

    mActionNewLayer = new QAction(this);
    mActionNewLayer->setIcon(QIcon(QLatin1String(":/images/16/document-new.png")));
    connect(mActionNewLayer, &QAction::triggered,
            handler->actionAddObjectGroup(), &QAction::trigger);

    QIcon objectsGroupIcon(QLatin1String("://images/16/layer-object.png"));
    objectsGroupIcon.addFile(QLatin1String("://images/32/layer-object.png"));

    mActionMoveToGroup = new QAction(this);
    mActionMoveToGroup->setIcon(objectsGroupIcon);

    mActionMoveUp = new QAction(this);
    mActionMoveUp->setIcon(QIcon(QLatin1String(":/images/16/go-up.png")));
    mActionMoveDown = new QAction(this);
    mActionMoveDown->setIcon(QIcon(QLatin1String(":/images/16/go-down.png")));

    Utils::setThemeIcon(mActionObjectProperties, "document-properties");
    Utils::setThemeIcon(mActionMoveUp, "go-up");
    Utils::setThemeIcon(mActionMoveDown, "go-down");

    auto *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(Utils::smallIconSize());

    toolBar->addAction(mActionNewLayer);
    toolBar->addAction(handler->actionDuplicateObjects());
    toolBar->addAction(handler->actionRemoveObjects());

    toolBar->addAction(mActionMoveUp);
    toolBar->addAction(mActionMoveDown);
    toolBar->addAction(mActionMoveToGroup);
    QToolButton *button;
    button = dynamic_cast<QToolButton*>(toolBar->widgetForAction(mActionMoveToGroup));
    mMoveToMenu = new QMenu(this);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setMenu(mMoveToMenu);
    connect(mMoveToMenu, &QMenu::aboutToShow, this, &ObjectsDock::aboutToShowMoveToMenu);
    connect(mMoveToMenu, &QMenu::triggered, this, &ObjectsDock::triggeredMoveToMenu);

    toolBar->addAction(mActionObjectProperties);

    layout->addWidget(toolBar);
    setWidget(widget);
    retranslateUi();

    ActionManager::registerAction(mActionMoveUp, "MoveObjectsUp");
    ActionManager::registerAction(mActionMoveDown, "MoveObjectsDown");

    connect(mActionMoveUp, &QAction::triggered, this, &ObjectsDock::moveObjectsUp);
    connect(mActionMoveDown, &QAction::triggered, this, &ObjectsDock::moveObjectsDown);
}

void ObjectsDock::moveObjectsUp()
{
    if (mMapDocument)
        mMapDocument->moveObjectsUp(mMapDocument->selectedObjects());
}

void ObjectsDock::moveObjectsDown()
{
    if (mMapDocument)
        mMapDocument->moveObjectsDown(mMapDocument->selectedObjects());
}

void ObjectsDock::setMapDocument(MapDocument *mapDoc)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDoc;

    mObjectsView->setMapDocument(mapDoc);

    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::selectedObjectsChanged,
                this, &ObjectsDock::updateActions);
    }

    updateActions();
}

void ObjectsDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void ObjectsDock::retranslateUi()
{
    setWindowTitle(tr("Objects"));

    mFilterEdit->setPlaceholderText(tr("Filter"));

    mActionNewLayer->setText(tr("Add Object Layer"));
    mActionObjectProperties->setText(tr("Object Properties"));
    mActionMoveUp->setText(tr("Move Objects Up"));
    mActionMoveDown->setText(tr("Move Objects Down"));

    updateActions();
}

void ObjectsDock::updateActions()
{
    int selectedObjectsCount = 0;
    int objectGroupCount = 0;

    if (mMapDocument) {
        selectedObjectsCount = mMapDocument->selectedObjects().count();
        objectGroupCount = mMapDocument->map()->objectGroupCount();
    }

    mActionObjectProperties->setEnabled(selectedObjectsCount > 0);
    mActionMoveToGroup->setEnabled(selectedObjectsCount > 0 && objectGroupCount >= 2);
    mActionMoveToGroup->setToolTip(tr("Move %n Object(s) to Layer", "", selectedObjectsCount));
    mActionMoveUp->setEnabled(selectedObjectsCount > 0);
    mActionMoveDown->setEnabled(selectedObjectsCount > 0);
}

void ObjectsDock::aboutToShowMoveToMenu()
{
    mMoveToMenu->clear();

    auto &selectedObjects = mMapDocument->selectedObjects();
    auto sameObjectGroup = RaiseLowerHelper::sameObjectGroup(selectedObjects);

    auto handler = MapDocumentActionHandler::instance();
    handler->populateMoveToLayerMenu(mMoveToMenu, sameObjectGroup);
}

void ObjectsDock::triggeredMoveToMenu(QAction *action)
{
    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    auto *objectGroup = action->data().value<ObjectGroup*>();
    handler->moveObjectsToGroup(objectGroup);
}

void ObjectsDock::objectProperties()
{
    const auto &selectedObjects = mMapDocument->selectedObjects();
    mMapDocument->setCurrentObject(selectedObjects.first());
    emit mMapDocument->editCurrentObject();
}

#include "moc_objectsdock.cpp"
