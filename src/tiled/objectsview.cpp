/*
 * objectsview.cpp
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
 * Copyright 2012-2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "objectsview.h"

#include "iconcheckdelegate.h"
#include "mapdocument.h"
#include "mapobjectmodel.h"
#include "preferences.h"
#include "reversingproxymodel.h"
#include "utils.h"

#include <QAction>
#include <QGuiApplication>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QSettings>

namespace Tiled {

static const char FIRST_COLUMN_WIDTH_KEY[] = "ObjectsDock/FirstSectionSize";
static const char VISIBLE_COLUMNS_KEY[] = "ObjectsDock/VisibleSections";

class ObjectsFilterModel : public ReversingProxyModel
{
    Q_OBJECT

public:
    ObjectsFilterModel(QObject *parent = nullptr)
        : ReversingProxyModel(parent)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        setRecursiveFilteringEnabled(true);
#endif
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        return filterRecursiveAcceptsRow(sourceRow, sourceParent);
    }

private:
    bool filterRecursiveAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (ReversingProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return true;

        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const int count = sourceModel()->rowCount(index);

        for (int i = 0; i < count; ++i)
            if (filterRecursiveAcceptsRow(i, index))
                return true;

        return false;
    }
#endif
};

ObjectsView::ObjectsView(QWidget *parent)
    : QTreeView(parent)
    , mProxyModel(new ObjectsFilterModel(this))
{
    setMouseTracking(true);

    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setFilterKeyColumn(-1);

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

        if (mActiveFilter)
            expandAll();
    } else {
        mProxyModel->setSourceModel(nullptr);
    }
}

MapObjectModel *ObjectsView::mapObjectModel() const
{
    return mMapDocument ? mMapDocument->mapObjectModel() : nullptr;
}

QModelIndex ObjectsView::layerViewIndex(Layer *layer) const
{
    if (MapObjectModel *model = mapObjectModel()) {
        QModelIndex sourceIndex = model->index(layer);
        return mProxyModel->mapFromSource(sourceIndex);
    }

    return QModelIndex();
}

void ObjectsView::ensureVisible(MapObject *mapObject)
{
    scrollTo(mProxyModel->mapFromSource(mapObjectModel()->index(mapObject)));
}

void ObjectsView::setFilter(const QString &filter)
{
    const bool hadActiveFilter = mActiveFilter;
    const bool activeFilter = !filter.isEmpty();

    if (!hadActiveFilter && activeFilter)
        saveExpandedLayers();

    mProxyModel->setFilterFixedString(filter);
    mActiveFilter = activeFilter;

    if (activeFilter) {
        expandAll();        // Expand to see all results
    } else if (hadActiveFilter) {
        collapseAll();
        restoreExpandedLayers();
        expandToSelectedObjects();
    }
}

void ObjectsView::saveExpandedLayers()
{
    if (mActiveFilter)
        return;

    mExpandedLayers[mMapDocument].clear();

    for (Layer *layer : mMapDocument->map()->allLayers()) {
        if (!layer->isObjectGroup() && !layer->isGroupLayer())
            continue;

        const QModelIndex sourceIndex = mMapDocument->mapObjectModel()->index(layer);
        const QModelIndex index = mProxyModel->mapFromSource(sourceIndex);
        if (isExpanded(index))
            mExpandedLayers[mMapDocument].append(layer);
    }
}

void ObjectsView::restoreExpandedLayers()
{
    if (mActiveFilter)
        return;

    const auto layers = mExpandedLayers.take(mMapDocument);
    for (Layer *layer : layers) {
        const QModelIndex sourceIndex = mMapDocument->mapObjectModel()->index(layer);
        const QModelIndex index = mProxyModel->mapFromSource(sourceIndex);
        setExpanded(index, true);
    }
}

void ObjectsView::clearExpandedLayers(MapDocument *mapDocument)
{
    mExpandedLayers.remove(mapDocument);
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
            emit mMapDocument->focusMapObjectRequested(mapObject);

    } else if (Layer *layer = mapObjectModel()->toLayer(index)) {
        mMapDocument->setCurrentObject(layer);
        mMapDocument->switchSelectedLayers({ layer });
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

void ObjectsView::expandToSelectedObjects()
{
    const QList<MapObject *> &selectedObjects = mMapDocument->selectedObjects();
    for (auto object : selectedObjects) {
        auto index = mProxyModel->mapFromSource(mapObjectModel()->index(object));

        // Make sure all parents are expanded
        for (QModelIndex parent = index.parent(); parent.isValid(); parent = parent.parent())
            if (!isExpanded(parent))
                expand(parent);
    }
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

} // namespace Tiled

#include "objectsview.moc"
