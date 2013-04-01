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

#include "documentmanager.h"
#include "map.h"
#include "mapobject.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
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

    mActionObjectProperties = new QAction(this);
    mActionObjectProperties->setIcon(QIcon(QLatin1String(":/images/16x16/document-properties.png")));

    Utils::setThemeIcon(mActionObjectProperties, "document-properties");

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    connect(mActionObjectProperties, SIGNAL(triggered()), SLOT(objectProperties()));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);
    layout->setSpacing(0);
    layout->addWidget(mObjectsView);

    mActionNewLayer = new QAction(this);
    mActionNewLayer->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));
    connect(mActionNewLayer, SIGNAL(triggered()),
            handler->actionAddObjectGroup(), SIGNAL(triggered()));

    mActionMoveToGroup = new QAction(this);
    mActionMoveToGroup->setIcon(QIcon(QLatin1String(":/images/16x16/layer-object.png")));

    QToolBar *toolbar = new QToolBar;
    toolbar->setFloatable(false);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(16, 16));

    toolbar->addAction(mActionNewLayer);
    toolbar->addAction(handler->actionDuplicateObjects());
    toolbar->addAction(handler->actionRemoveObjects());

    toolbar->addAction(mActionMoveToGroup);
    QToolButton *button;
    button = dynamic_cast<QToolButton*>(toolbar->widgetForAction(mActionMoveToGroup));
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

    mActionNewLayer->setToolTip(tr("Add Object Layer"));
    mActionObjectProperties->setToolTip(tr("Object Properties"));

    updateActions();
}

void ObjectsDock::updateActions()
{
    int count = mMapDocument ? mMapDocument->selectedObjects().count() : 0;
    bool enabled = count > 0;
    mActionObjectProperties->setEnabled(enabled && (count == 1));

    if (mMapDocument && (mMapDocument->map()->objectGroupCount() < 2))
        enabled = false;
    mActionMoveToGroup->setEnabled(enabled);
    mActionMoveToGroup->setToolTip(tr("Move %n Object(s) to Layer", "", count));
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
    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    ObjectGroup *objectGroup = action->data().value<ObjectGroup*>();
    handler->moveObjectsToGroup(objectGroup);
}

void ObjectsDock::objectProperties()
{
    // Unnecessary check is unnecessary
    if (!mMapDocument || !mMapDocument->selectedObjects().count())
        return;

    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();

    MapObject *mapObject = selectedObjects.first();
    ObjectPropertiesDialog propertiesDialog(mMapDocument, mapObject, this);
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

        // 2 equal-sized columns, user can't adjust
#if QT_VERSION >= 0x050000
        header()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
        header()->setResizeMode(0, QHeaderView::Stretch);
#endif

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
    if (MapObject *mapObject = model()->toMapObject(index)) {
        ObjectPropertiesDialog propertiesDialog(mMapDocument, mapObject, this);
        propertiesDialog.exec();
    }
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
            const MapObject *o = selectedObjects.first();
            const QPointF center = o->bounds().center();
            DocumentManager::instance()->centerViewOn(center);
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
