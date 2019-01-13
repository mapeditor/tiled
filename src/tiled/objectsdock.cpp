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
#include "grouplayer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapobject.h"
#include "mapobjectmodel.h"
#include "objectgroup.h"
#include "preferences.h"
#include "reversingproxymodel.h"
#include "utils.h"
#include "iconcheckdelegate.h"

#include <QApplication>
#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>

static const char FIRST_COLUMN_WIDTH_KEY[] = "ObjectsDock/FirstSectionSize";
static const char VISIBLE_COLUMNS_KEY[] = "ObjectsDock/VisibleSections";

using namespace Tiled;

ObjectsDock::ObjectsDock(QWidget *parent)
    : QDockWidget(parent)
    , mObjectsView(new ObjectsView)
    , mMapDocument(nullptr)
{
    setObjectName(QLatin1String("ObjectsDock"));

    mActionObjectProperties = new QAction(this);
    mActionObjectProperties->setIcon(QIcon(QLatin1String(":/images/16x16/document-properties.png")));

    connect(mActionObjectProperties, &QAction::triggered,
            this, &ObjectsDock::objectProperties);

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mObjectsView);

    mActionNewLayer = new QAction(this);
    mActionNewLayer->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));
    connect(mActionNewLayer, &QAction::triggered,
            handler->actionAddObjectGroup(), &QAction::trigger);

    mActionMoveToGroup = new QAction(this);
    mActionMoveToGroup->setIcon(QIcon(QLatin1String(":/images/16x16/layer-object.png")));

    mActionMoveUp = new QAction(this);
    mActionMoveUp->setIcon(QIcon(QLatin1String(":/images/16x16/go-up.png")));
    mActionMoveDown = new QAction(this);
    mActionMoveDown->setIcon(QIcon(QLatin1String(":/images/16x16/go-down.png")));

    Utils::setThemeIcon(mActionObjectProperties, "document-properties");
    Utils::setThemeIcon(mActionMoveUp, "go-up");
    Utils::setThemeIcon(mActionMoveDown, "go-down");

    QToolBar *toolBar = new QToolBar;
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

    connect(DocumentManager::instance(), &DocumentManager::documentAboutToClose,
            this, &ObjectsDock::documentAboutToClose);

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
    if (mMapDocument) {
        saveExpandedGroups();
        mMapDocument->disconnect(this);
    }

    mMapDocument = mapDoc;

    mObjectsView->setMapDocument(mapDoc);

    if (mMapDocument) {
        restoreExpandedGroups();
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

    mActionNewLayer->setToolTip(tr("Add Object Layer"));
    mActionObjectProperties->setToolTip(tr("Object Properties"));
    mActionMoveUp->setToolTip(tr("Move Objects Up"));
    mActionMoveDown->setToolTip(tr("Move Objects Down"));

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

    for (Layer *layer : mMapDocument->map()->objectGroups()) {
        QAction *action = mMoveToMenu->addAction(layer->name());
        action->setData(QVariant::fromValue(static_cast<ObjectGroup*>(layer)));
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
    const auto &selectedObjects = mMapDocument->selectedObjects();
    mMapDocument->setCurrentObject(selectedObjects.first());
    emit mMapDocument->editCurrentObject();
}

void ObjectsDock::saveExpandedGroups()
{
    mExpandedGroups[mMapDocument].clear();

    const auto proxyModel = static_cast<QAbstractProxyModel*>(mObjectsView->model());

    for (Layer *layer : mMapDocument->map()->objectGroups()) {
        const QModelIndex sourceIndex = mMapDocument->mapObjectModel()->index(layer);
        const QModelIndex index = proxyModel->mapFromSource(sourceIndex);
        if (mObjectsView->isExpanded(index))
            mExpandedGroups[mMapDocument].append(layer);
    }
}

void ObjectsDock::restoreExpandedGroups()
{
    const auto objectGroups = mExpandedGroups.take(mMapDocument);
    for (Layer *layer : objectGroups) {
        const QModelIndex sourceIndex = mMapDocument->mapObjectModel()->index(layer);
        const QModelIndex index = static_cast<QAbstractProxyModel*>(mObjectsView->model())->mapFromSource(sourceIndex);
        mObjectsView->setExpanded(index, true);
    }
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
    , mProxyModel(new ReversingProxyModel(this))
    , mSynching(false)
{
    setMouseTracking(true);

    setUniformRowHeights(true);
    setModel(mProxyModel);
    setItemDelegate(new IconCheckDelegate(IconCheckDelegate::VisibilityIcon, false, this));

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, &QAbstractItemView::activated, this, &ObjectsView::onActivated);

    connect(header(), &QHeaderView::sectionResized,
            this, &ObjectsView::onSectionResized);

    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header(), &QWidget::customContextMenuRequested,
            this, &ObjectsView::showCustomHeaderContextMenu);
}

QSize ObjectsView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void ObjectsView::setMapDocument(MapDocument *mapDoc)
{
    if (mapDoc == mMapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDoc;

    if (mMapDocument) {
        mProxyModel->setSourceModel(mMapDocument->mapObjectModel());

        const QSettings *settings = Preferences::instance()->settings();
        const int firstColumnWidth =
                settings->value(QLatin1String(FIRST_COLUMN_WIDTH_KEY), 200).toInt();
        setColumnWidth(0, firstColumnWidth);

        connect(mMapDocument, &MapDocument::selectedObjectsChanged,
                this, &ObjectsView::selectedObjectsChanged);

        connect(mMapDocument, &MapDocument::hoveredMapObjectChanged,
                this, &ObjectsView::hoveredObjectChanged);

        restoreVisibleColumns();
        synchronizeSelectedItems();
    } else {
        mProxyModel->setSourceModel(nullptr);
    }
}

MapObjectModel *ObjectsView::mapObjectModel() const
{
    return mMapDocument ? mMapDocument->mapObjectModel() : nullptr;
}

bool ObjectsView::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Tab) {
            if (indexWidget(currentIndex())) {
                event->accept();
                return true;
            }
        }
    }

    return QTreeView::event(event);
}

void ObjectsView::mousePressEvent(QMouseEvent *event)
{
    const QModelIndex proxyIndex = indexAt(event->pos());
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);

    if (MapObject *mapObject = mapObjectModel()->toMapObject(index)) {
        mMapDocument->setCurrentObject(mapObject);

        if (event->button() == Qt::LeftButton && !event->modifiers())
            mMapDocument->focusMapObjectRequested(mapObject);

    } else if (Layer *layer = mapObjectModel()->toLayer(index)) {
        mMapDocument->setCurrentObject(layer);
        mMapDocument->setCurrentLayer(layer);
    }

    QTreeView::mousePressEvent(event);
}

void ObjectsView::mouseMoveEvent(QMouseEvent *event)
{
    if (!mMapDocument)
        return;

    const QModelIndex proxyIndex = indexAt(event->pos());
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);

    MapObject *mapObject = mapObjectModel()->toMapObject(index);
    mMapDocument->setHoveredMapObject(mapObject);
}

bool ObjectsView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::Leave) {
        if (mMapDocument)
            mMapDocument->setHoveredMapObject(nullptr);
    }

    return QTreeView::viewportEvent(event);
}

void ObjectsView::onActivated(const QModelIndex &proxyIndex)
{
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);

    if (MapObject *mapObject = mapObjectModel()->toMapObject(index)) {
        mMapDocument->setCurrentObject(mapObject);
        emit mMapDocument->focusMapObjectRequested(mapObject);
    }
}

void ObjectsView::onSectionResized(int logicalIndex)
{
    if (logicalIndex != 0)
        return;

    QSettings *settings = Preferences::instance()->settings();
    settings->setValue(QLatin1String(FIRST_COLUMN_WIDTH_KEY),
                       columnWidth(0));
}

void ObjectsView::selectionChanged(const QItemSelection &selected,
                                   const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    if (!mMapDocument || mSynching)
        return;

    const QModelIndexList selectedProxyRows = selectionModel()->selectedRows();

    QList<MapObject*> selectedObjects;
    for (const QModelIndex &proxyIndex : selectedProxyRows) {
        const QModelIndex index = mProxyModel->mapToSource(proxyIndex);

        if (MapObject *o = mapObjectModel()->toMapObject(index))
            selectedObjects.append(o);
    }

    if (selectedObjects != mMapDocument->selectedObjects()) {
        mSynching = true;
        mMapDocument->setSelectedObjects(selectedObjects);
        mSynching = false;
    }
}

void ObjectsView::drawRow(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &proxyIndex) const
{
    if (mMapDocument) {
        const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
        const MapObject *mapObject = mapObjectModel()->toMapObject(index);

        if (mapObject && mapObject == mMapDocument->hoveredMapObject()) {
            QColor hoverColor = QGuiApplication::palette().highlight().color();
            hoverColor.setAlpha(64);
            painter->fillRect(option.rect, hoverColor);
        }
    }

    QTreeView::drawRow(painter, option, proxyIndex);
}

void ObjectsView::selectedObjectsChanged()
{
    if (mSynching)
        return;

    synchronizeSelectedItems();

    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();
    if (selectedObjects.count() == 1) {
        MapObject *o = selectedObjects.first();
        scrollTo(mProxyModel->mapFromSource(mapObjectModel()->index(o)));
    }
}

void ObjectsView::hoveredObjectChanged(MapObject *object, MapObject *previous)
{
    updateRow(object);
    updateRow(previous);
}

void ObjectsView::setColumnVisibility(bool visible)
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    int column = action->data().toInt();
    setColumnHidden(column, !visible);

    QSettings *settings = Preferences::instance()->settings();
    QVariantList visibleColumns;
    for (int i = 0; i < mProxyModel->columnCount(); i++) {
        if (!isColumnHidden(i))
            visibleColumns.append(i);
    }
    settings->setValue(QLatin1String(VISIBLE_COLUMNS_KEY), visibleColumns);
}

void ObjectsView::showCustomHeaderContextMenu(const QPoint &point)
{
    Q_UNUSED(point)
    QMenu contextMenu(this);
    QAbstractItemModel *model = mProxyModel->sourceModel();
    for (int i = 0; i < model->columnCount(); i++) {
        if (i == MapObjectModel::Name)
            continue;
        QAction *action = new QAction(model->headerData(i, Qt::Horizontal).toString(), &contextMenu);
        action->setCheckable(true);
        action->setChecked(!isColumnHidden(i));
        action->setData(i);
        connect(action, &QAction::triggered, this, &ObjectsView::setColumnVisibility);
        contextMenu.addAction(action);
    }
    contextMenu.exec(QCursor::pos());
}

void ObjectsView::restoreVisibleColumns()
{
    QSettings *settings = Preferences::instance()->settings();
    QVariantList visibleColumns = settings->value(QLatin1String(VISIBLE_COLUMNS_KEY),
                                                  QVariantList() << MapObjectModel::Name << MapObjectModel::Type).toList();

    for (int i = 0; i < mProxyModel->columnCount(); i++)
        setColumnHidden(i, !visibleColumns.contains(i));
}

void ObjectsView::synchronizeSelectedItems()
{
    Q_ASSERT(!mSynching);
    Q_ASSERT(mMapDocument);

    QItemSelection itemSelection;

    for (MapObject *o : mMapDocument->selectedObjects()) {
        QModelIndex index = mProxyModel->mapFromSource(mapObjectModel()->index(o));
        itemSelection.select(index, index);
    }

    mSynching = true;
    selectionModel()->select(itemSelection,
                             QItemSelectionModel::Select |
                             QItemSelectionModel::Rows |
                             QItemSelectionModel::Clear);
    mSynching = false;
}

void ObjectsView::updateRow(MapObject *object)
{
    if (!object || !object->objectGroup())
        return;

    const QModelIndex index = mapObjectModel()->index(object);
    const QModelIndex proxyIndex = mProxyModel->mapFromSource(index);
    const QRect rect = visualRect(proxyIndex);

    viewport()->update(QRect(0, rect.y(), viewport()->width(), rect.height()));
}
