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

#include "addremovemapobject.h"
#include "documentmanager.h"
#include "map.h"
#include "mapobject.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "movemapobjecttogroup.h"
#include "objectgroup.h"
#include "objectpropertiesdialog.h"
#include "utils.h"
#include "mapobjectmodel.h"

#include <QBoxLayout>
#include <QApplication>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QMenu>
#include <QSlider>
#include <QToolBar>
#include <QUrl>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

ObjectsDock::ObjectsDock(QWidget *parent)
    : QDockWidget(parent)
    , mObjectsView(new ObjectsView())
    , mMapDocument(0)
{
    setObjectName(QLatin1String("ObjectsDock"));

    mActionDuplicateObjects = new QAction(this);
    mActionDuplicateObjects->setIcon(QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));

    mActionRemoveObjects = new QAction(this);
    mActionRemoveObjects->setIcon(QIcon(QLatin1String(":/images/16x16/edit-delete.png")));

    mActionObjectProperties = new QAction(this);
    mActionObjectProperties->setIcon(QIcon(QLatin1String(":/images/16x16/document-properties.png")));
    mActionObjectProperties->setToolTip(tr("Object Propertes"));

    Utils::setThemeIcon(mActionRemoveObjects, "edit-delete");
    Utils::setThemeIcon(mActionObjectProperties, "document-properties");

    connect(mActionDuplicateObjects, SIGNAL(triggered()), SLOT(duplicateObjects()));
    connect(mActionRemoveObjects, SIGNAL(triggered()), SLOT(removeObjects()));
    connect(mActionObjectProperties, SIGNAL(triggered()), SLOT(objectProperties()));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);
    layout->addWidget(mObjectsView);

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QAction *newLayerAction = new QAction(this);
    newLayerAction->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));
    newLayerAction->setToolTip(tr("Add Object Layer"));
    connect(newLayerAction, SIGNAL(triggered()),
            handler->actionAddObjectGroup(), SIGNAL(triggered()));

    mActionMoveToLayer = new QAction(this);
    mActionMoveToLayer->setIcon(QIcon(QLatin1String(":/images/16x16/layer-object.png")));
    mActionMoveToLayer->setToolTip(tr("Move Object To Layer"));

    QToolBar *toolbar = new QToolBar;
    toolbar->setFloatable(false);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(16, 16));

    toolbar->addAction(newLayerAction);
    toolbar->addAction(mActionDuplicateObjects);
    toolbar->addAction(mActionRemoveObjects);

    toolbar->addAction(mActionMoveToLayer);
    QToolButton *button;
    button = dynamic_cast<QToolButton*>(toolbar->widgetForAction(mActionMoveToLayer));
    mMoveToMenu = new QMenu(this);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setMenu(mMoveToMenu);
    connect(mMoveToMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowMoveToMenu()));
    connect(mMoveToMenu, SIGNAL(triggered(QAction*)),
            SLOT(triggeredMoveToMenu(QAction*)));

    toolbar->addAction(mActionObjectProperties);

    layout->addWidget(toolbar);
    setWidget(widget);
    retranslateUi();

    // Workaround since a tabbed dockwidget that is not currently visible still
    // returns true for isVisible()
    connect(this, SIGNAL(visibilityChanged(bool)),
            mObjectsView, SLOT(setVisible(bool)));

    connect(DocumentManager::instance(), SIGNAL(documentCloseRequested(int)),
            SLOT(documentCloseRequested(int)));

    updateActions();
}

void ObjectsDock::setMapDocument(MapDocument *mapDoc)
{
    if (mMapDocument) {
        saveExpandedGroups(mMapDocument);
        mMapDocument->disconnect(this);
    }

    mMapDocument = mapDoc;

    mObjectsView->setMapDocument(mapDoc);

    if (mMapDocument) {
        restoreExpandedGroups(mMapDocument);
        connect(mMapDocument, SIGNAL(selectedObjectsChanged()),
                this, SLOT(updateActions()));
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
}

void ObjectsDock::updateActions()
{
    int count = mMapDocument ? mMapDocument->selectedObjects().count() : 0;
    bool enabled = count > 0;
    mActionDuplicateObjects->setEnabled(enabled);
    mActionRemoveObjects->setEnabled(enabled);
    mActionObjectProperties->setEnabled(enabled && (count == 1));

    mActionDuplicateObjects->setToolTip((enabled && count > 1)
        ? tr("Duplicate %n Objects", "", count) : tr("Duplicate Object"));
    mActionRemoveObjects->setToolTip((enabled && count > 1)
        ? tr("Remove %n Objects", "", count) : tr("Remove Object"));

    if (mMapDocument && (mMapDocument->map()->objectGroupCount() < 2))
        enabled = false;
    mActionMoveToLayer->setEnabled(enabled);
    mActionMoveToLayer->setToolTip((enabled && count > 1)
        ? tr("Move %n Objects To Layer", "", count) : tr("Move Object To Layer"));
}

void ObjectsDock::aboutToShowMoveToMenu()
{
    mMoveToMenu->clear();

    foreach (ObjectGroup *objectGroup, mMapDocument->map()->objectGroups()) {
        QAction *action = mMoveToMenu->addAction(objectGroup->name());
        action->setData(QVariant::fromValue(objectGroup));
    }
}

void ObjectsDock::triggeredMoveToMenu(QAction *action)
{
    ObjectGroup *objectGroup = action->data().value<ObjectGroup*>();

    const QList<MapObject *> &objects = mMapDocument->selectedObjects();

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s) to Layer", "",
                             objects.size()));
    foreach (MapObject *mapObject, objects) {
        if (mapObject->objectGroup() == objectGroup)
            continue;
        undoStack->push(new MoveMapObjectToGroup(mMapDocument,
                                                 mapObject,
                                                 objectGroup));
    }
    undoStack->endMacro();
}

void ObjectsDock::duplicateObjects()
{
    // Unnecessary check is unnecessary
    if (!mMapDocument || !mMapDocument->selectedObjects().count())
        return;

    const QList<MapObject *> &objects = mMapDocument->selectedObjects();

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Duplicate %n Object(s)", "", objects.size()));

    QList<MapObject*> clones;
    foreach (MapObject *mapObject, objects) {
        MapObject *clone = mapObject->clone();
        undoStack->push(new AddMapObject(mMapDocument,
                                         mapObject->objectGroup(),
                                         clone));
        clones << clone;
    }

    undoStack->endMacro();
    mMapDocument->setSelectedObjects(clones);
}

void ObjectsDock::removeObjects()
{
    // Unnecessary check is unnecessary
    if (!mMapDocument || !mMapDocument->selectedObjects().count())
        return;

    const QList<MapObject *> &objects = mMapDocument->selectedObjects();

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Remove %n Object(s)", "", objects.size()));
    foreach (MapObject *mapObject, objects)
        undoStack->push(new RemoveMapObject(mMapDocument, mapObject));
    undoStack->endMacro();
}

void ObjectsDock::objectProperties()
{
    // Unnecessary check is unnecessary
    if (!mMapDocument || !mMapDocument->selectedObjects().count())
        return;

    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();

    MapObject *mapObject = selectedObjects.first();
    ObjectPropertiesDialog propertiesDialog(mMapDocument, mapObject, 0);
    propertiesDialog.exec();
}

void ObjectsDock::saveExpandedGroups(MapDocument *mapDoc)
{
    mExpandedGroups[mapDoc].clear();
    foreach (ObjectGroup *og, mapDoc->map()->objectGroups()) {
        if (mObjectsView->isExpanded(mObjectsView->model()->index(og)))
            mExpandedGroups[mapDoc].append(og);
    }
}

void ObjectsDock::restoreExpandedGroups(MapDocument *mapDoc)
{
    foreach (ObjectGroup *og, mExpandedGroups[mapDoc])
        mObjectsView->setExpanded(mObjectsView->model()->index(og), true);
    mExpandedGroups[mapDoc].clear();

    // Also restore the selection
    foreach (MapObject *o, mapDoc->selectedObjects()) {
        QModelIndex index = mObjectsView->model()->index(o);
        mObjectsView->selectionModel()->select(index, QItemSelectionModel::Select |  QItemSelectionModel::Rows);
    }
}

void ObjectsDock::documentCloseRequested(int index)
{
    DocumentManager *documentManager = DocumentManager::instance();
    MapDocument *mapDoc = documentManager->documents().at(index);
    mExpandedGroups.remove(mapDoc);
}

///// ///// ///// ///// /////

ObjectsView::ObjectsView(QWidget *parent)
    : QTreeView(parent)
    , mMapDocument(0)
    , mSynching(false)
{
    setRootIsDecorated(true);
    setHeaderHidden(false);
    setItemsExpandable(true);
    setUniformRowHeights(true);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, SIGNAL(activated(QModelIndex)), SLOT(onActivated(QModelIndex)));
}

QSize ObjectsView::sizeHint() const
{
    return QSize(130, 100);
}

void ObjectsView::setMapDocument(MapDocument *mapDoc)
{
    if (mapDoc == mMapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDoc;

    if (mMapDocument) {
        mMapObjectModel = mMapDocument->mapObjectModel();
        setModel(mMapObjectModel);
        model()->setMapDocument(mapDoc);
        header()->setResizeMode(0, QHeaderView::Stretch); // 2 equal-sized columns, user can't adjust

        connect(mMapDocument, SIGNAL(selectedObjectsChanged()),
                this, SLOT(selectedObjectsChanged()));
    } else {
        if (model())
            model()->setMapDocument(0);
        setModel(mMapObjectModel = 0);
    }

}

void ObjectsView::onActivated(const QModelIndex &index)
{
    Q_UNUSED(index)
    // show object properties, center in view
}

void ObjectsView::selectionChanged(const QItemSelection &selected,
                                   const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    if (!mMapDocument || mSynching)
        return;

    QModelIndexList selectedRows = selectionModel()->selectedRows();
    int currentLayerIndex = -1;

    QList<MapObject*> selectedObjects;
    foreach (const QModelIndex &index, selectedRows) {
        if (ObjectGroup *og = model()->toLayer(index)) {
            int index = mMapDocument->map()->layers().indexOf(og);
            if (currentLayerIndex == -1)
                currentLayerIndex = index;
            else if (currentLayerIndex != index)
                currentLayerIndex = -2;
        }
        if (MapObject *o = model()->toMapObject(index))
            selectedObjects.append(o);
    }

    // Switch the current object layer if only one object layer (and/or its objects)
    // are included in the current selection.
    if (currentLayerIndex >= 0 && currentLayerIndex != mMapDocument->currentLayerIndex())
        mMapDocument->setCurrentLayerIndex(currentLayerIndex);

    if (selectedObjects != mMapDocument->selectedObjects()) {
        mSynching = true;
        if (selectedObjects.count() == 1) {
            MapObject *o = selectedObjects.first();
            QPoint pos = o->position().toPoint();
            QSize size = o->size().toSize();
            DocumentManager::instance()->centerViewOn(pos.x() + size.width() / 2,
                                                      pos.y() + size.height() / 2);
        }
        mMapDocument->setSelectedObjects(selectedObjects);
        mSynching = false;
    }
}

void ObjectsView::selectedObjectsChanged()
{
    if (mSynching)
        return;

    if (!mMapDocument)
        return;

    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();

    mSynching = true;
    clearSelection();
    foreach (MapObject *o, selectedObjects) {
        QModelIndex index = model()->index(o);
        selectionModel()->select(index, QItemSelectionModel::Select |  QItemSelectionModel::Rows);
    }
    mSynching = false;

    if (selectedObjects.count() == 1) {
        MapObject *o = selectedObjects.first();
        scrollTo(model()->index(o));
    }
}
