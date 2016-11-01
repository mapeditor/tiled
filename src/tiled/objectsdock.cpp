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
#include "mapobjectmodel.h"
#include "objectgroup.h"
#include "preferences.h"
#include "utils.h"

#include <QApplication>
#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QSettings>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>

static const char FIRST_SECTION_SIZE_KEY[] = "ObjectsDock/FirstSectionSize";

using namespace Tiled;
using namespace Tiled::Internal;

ObjectsDock::ObjectsDock(QWidget *parent)
    : QDockWidget(parent)
    , mObjectsView(new ObjectsView)
    , mMapDocument(nullptr)
{
    setObjectName(QLatin1String("ObjectsDock"));

    mActionObjectProperties = new QAction(this);
    mActionObjectProperties->setIcon(QIcon(QLatin1String(":/images/16x16/document-properties.png")));

    Utils::setThemeIcon(mActionObjectProperties, "document-properties");

    connect(mActionObjectProperties, SIGNAL(triggered()), SLOT(objectProperties()));

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mObjectsView);

    mActionNewLayer = new QAction(this);
    mActionNewLayer->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));
    connect(mActionNewLayer, SIGNAL(triggered()),
            handler->actionAddObjectGroup(), SIGNAL(triggered()));

    mActionMoveToGroup = new QAction(this);
    mActionMoveToGroup->setIcon(QIcon(QLatin1String(":/images/16x16/layer-object.png")));

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(16, 16));

    toolBar->addAction(mActionNewLayer);
    toolBar->addAction(handler->actionDuplicateObjects());
    toolBar->addAction(handler->actionRemoveObjects());

    toolBar->addAction(mActionMoveToGroup);
    QToolButton *button;
    button = dynamic_cast<QToolButton*>(toolBar->widgetForAction(mActionMoveToGroup));
    mMoveToMenu = new QMenu(this);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setMenu(mMoveToMenu);
    connect(mMoveToMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowMoveToMenu()));
    connect(mMoveToMenu, SIGNAL(triggered(QAction*)),
            SLOT(triggeredMoveToMenu(QAction*)));

    toolBar->addAction(mActionObjectProperties);

    layout->addWidget(toolBar);
    setWidget(widget);
    retranslateUi();

    connect(DocumentManager::instance(), &DocumentManager::documentAboutToClose,
            this, &ObjectsDock::documentAboutToClose);
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
    mActionObjectProperties->setEnabled(count == 1);

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
    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();
    MapObject *mapObject = selectedObjects.first();
    mMapDocument->setCurrentObject(mapObject);
    emit mMapDocument->editCurrentObject();
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
}

void ObjectsDock::documentAboutToClose(Document *document)
{
    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document))
        mExpandedGroups.remove(mapDocument);
}

///// ///// ///// ///// /////

ObjectsView::ObjectsView(QWidget *parent)
    : QTreeView(parent)
    , mMapDocument(nullptr)
    , mSynching(false)
{
    setUniformRowHeights(true);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, SIGNAL(pressed(QModelIndex)), SLOT(onPressed(QModelIndex)));
    connect(this, SIGNAL(activated(QModelIndex)), SLOT(onActivated(QModelIndex)));

    connect(header(), SIGNAL(sectionResized(int,int,int)),
            this, SLOT(onSectionResized(int)));
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
        setModel(mMapDocument->mapObjectModel());

        const QSettings *settings = Preferences::instance()->settings();
        const int firstSectionSize =
                settings->value(QLatin1String(FIRST_SECTION_SIZE_KEY), 200).toInt();
        header()->resizeSection(0, firstSectionSize);

        connect(mMapDocument, SIGNAL(selectedObjectsChanged()),
                this, SLOT(selectedObjectsChanged()));

        synchronizeSelectedItems();
    } else {
        setModel(nullptr);
    }
}

MapObjectModel *ObjectsView::model() const
{
    return static_cast<MapObjectModel*>(QTreeView::model());
}

void ObjectsView::onPressed(const QModelIndex &index)
{
    if (MapObject *mapObject = model()->toMapObject(index))
        mMapDocument->setCurrentObject(mapObject);
    else if (ObjectGroup *objectGroup = model()->toObjectGroup(index))
        mMapDocument->setCurrentObject(objectGroup);
}

void ObjectsView::onActivated(const QModelIndex &index)
{
    if (MapObject *mapObject = model()->toMapObject(index)) {
        mMapDocument->setCurrentObject(mapObject);
        emit mMapDocument->editCurrentObject();
    }
}

void ObjectsView::onSectionResized(int logicalIndex)
{
    if (logicalIndex != 0)
        return;

    QSettings *settings = Preferences::instance()->settings();
    settings->setValue(QLatin1String(FIRST_SECTION_SIZE_KEY),
                       header()->sectionSize(0));
}

void ObjectsView::selectionChanged(const QItemSelection &selected,
                                   const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    if (!mMapDocument || mSynching)
        return;

    const QModelIndexList selectedRows = selectionModel()->selectedRows();
    int currentLayerIndex = -1;

    QList<MapObject*> selectedObjects;
    for (const QModelIndex &index : selectedRows) {
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

    synchronizeSelectedItems();

    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();
    if (selectedObjects.count() == 1) {
        MapObject *o = selectedObjects.first();
        scrollTo(model()->index(o));
    }
}

void ObjectsView::synchronizeSelectedItems()
{
    Q_ASSERT(!mSynching);
    Q_ASSERT(mMapDocument);

    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();
    QItemSelection itemSelection;

    for (MapObject *o : selectedObjects) {
        QModelIndex index = model()->index(o);
        itemSelection.select(index, index);
    }

    mSynching = true;
    selectionModel()->select(itemSelection,
                             QItemSelectionModel::Select |
                             QItemSelectionModel::Rows |
                             QItemSelectionModel::Clear);
    mSynching = false;
}
