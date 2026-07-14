/*
 * abstractworldtool.cpp
 * Copyright 2019, Nils Kuebler <nils-kuebler@web.de>
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

#include "abstractworldtool.h"
#include "actionmanager.h"
#include "changeworld.h"
#include "documentmanager.h"
#include "mainwindow.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "preferences.h"
#include "selectionrectangle.h"
#include "utils.h"
#include "world.h"
#include "worlddocument.h"
#include "worldmanager.h"

#include <QAction>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>
#include <QtMath>

namespace Tiled {

namespace {

// A small square handle shown on a map's corners and edges for resizing
class MapResizeHandle : public QGraphicsItem
{
public:
    MapResizeHandle()
    {
        setAcceptedMouseButtons(Qt::MouseButtons());
        setAcceptHoverEvents(true);
        setFlag(QGraphicsItem::ItemIgnoresTransformations);
        setZValue(10000 + 1);
    }

    QRectF boundingRect() const override
    {
        return Utils::dpiScaled(QRectF(-5, -5, 10, 10));
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::white);
        painter->drawRect(Utils::dpiScaled(QRectF(-4, -4, 8, 8)));
    }
};

// The eight resize handle positions for bounds, in ResizeHandlePosition order,
// using a QRectF so right() and bottom() are not off by one like QRect
std::array<QPointF, HandleCount> resizeHandlePositions(const QRectF &bounds)
{
    const QPointF center = bounds.center();
    return {{
        bounds.topLeft(),    QPointF(center.x(), bounds.top()),    bounds.topRight(),
        QPointF(bounds.left(), center.y()),                        QPointF(bounds.right(), center.y()),
        bounds.bottomLeft(), QPointF(center.x(), bounds.bottom()), bounds.bottomRight(),
    }};
}

} // namespace

Qt::CursorShape cursorForHandle(int handle)
{
    switch (handle) {
    case TopLeftHandle: case BottomRightHandle: return Qt::SizeFDiagCursor;
    case TopRightHandle: case BottomLeftHandle: return Qt::SizeBDiagCursor;
    case TopHandle: case BottomHandle:          return Qt::SizeVerCursor;
    case LeftHandle: case RightHandle:          return Qt::SizeHorCursor;
    default:                                    return Qt::ArrowCursor;
    }
}

AbstractWorldTool::AbstractWorldTool(Id id,
                                     const QString &name,
                                     const QIcon &icon,
                                     const QKeySequence &shortcut,
                                     QObject *parent)
    : AbstractTool(id, name, icon, shortcut, parent)
    , mSelectionRectangle(new SelectionRectangle)
{
    mSelectionRectangle->setVisible(false);

    // Each handle owns its resize cursor, so it shows above a map's own cursor
    for (int i = 0; i < HandleCount; ++i) {
        mResizeHandles[i] = std::make_unique<MapResizeHandle>();
        mResizeHandles[i]->setVisible(false);
        mResizeHandles[i]->setCursor(cursorForHandle(i));
    }

    WorldManager &worldManager = WorldManager::instance();
    connect(&worldManager, &WorldManager::worldsChanged, this, &AbstractWorldTool::updateEnabledState);

    QIcon newWorldForMapIcon(QLatin1String(":images/24/world-map-add-other.png"));
    mNewWorldForMapAction = new QAction(this);
    mNewWorldForMapAction->setIcon(newWorldForMapIcon);
    ActionManager::registerAction(mNewWorldForMapAction, "NewWorldForMap");
    connect(mNewWorldForMapAction, &QAction::triggered, this, &AbstractWorldTool::createWorldForCurrentMap);

    QIcon addAnotherMapToWorldIcon(QLatin1String(":images/24/world-map-add-other.png"));
    mAddAnotherMapToWorldAction = new QAction(this);
    mAddAnotherMapToWorldAction->setIcon(addAnotherMapToWorldIcon);
    mAddAnotherMapToWorldAction->setShortcut(Qt::SHIFT + Qt::Key_A);
    ActionManager::registerAction(mAddAnotherMapToWorldAction, "AddAnotherMap");
    connect(mAddAnotherMapToWorldAction, &QAction::triggered, this, &AbstractWorldTool::addAnotherMapToWorldAtCenter);

    QIcon addMapToWorldIcon(QLatin1String(":images/24/world-map-add-this.png"));
    mAddMapToWorldAction = new QAction(this);
    mAddMapToWorldAction->setIcon(addMapToWorldIcon);
    mAddMapToWorldAction->setShortcut(Qt::SHIFT + Qt::Key_A);
    ActionManager::registerAction(mAddMapToWorldAction, "AddMap");

    QIcon removeMapFromWorldIcon(QLatin1String(":images/24/world-map-remove-this.png"));
    mRemoveMapFromWorldAction = new QAction(this);
    mRemoveMapFromWorldAction->setIcon(removeMapFromWorldIcon);
    mRemoveMapFromWorldAction->setShortcut(Qt::SHIFT + Qt::Key_D);
    ActionManager::registerAction(mRemoveMapFromWorldAction, "RemoveMap");
    connect(mRemoveMapFromWorldAction, &QAction::triggered, this, &AbstractWorldTool::removeCurrentMapFromWorld);

    languageChangedImpl();
}

AbstractWorldTool::~AbstractWorldTool() = default;

void AbstractWorldTool::activate(MapScene *scene)
{
    scene->addItem(mSelectionRectangle.get());
    for (auto &handle : mResizeHandles)
        scene->addItem(handle.get());
    connect(scene, &MapScene::sceneRefreshed, this, &AbstractWorldTool::updateSelectionRectangle);
    AbstractTool::activate(scene);
}

void AbstractWorldTool::deactivate(MapScene *scene)
{
    scene->removeItem(mSelectionRectangle.get());
    for (auto &handle : mResizeHandles)
        scene->removeItem(handle.get());
    disconnect(scene, &MapScene::sceneRefreshed, this, &AbstractWorldTool::updateSelectionRectangle);
    AbstractTool::deactivate(scene);
}

void AbstractWorldTool::mouseLeft()
{
    setStatusInfo(QString());
}

void AbstractWorldTool::mouseMoved(const QPointF &pos,
                                   Qt::KeyboardModifiers)
{
    setTargetMap(mapAt(pos));

    // Take into account the offset of the current layer
    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer())
        offsetPos -= mapScene()->absolutePositionForLayer(*layer);

    const QPoint pixelPos(qFloor(offsetPos.x()), qFloor(offsetPos.y()));
    const QPointF tilePosF = mapDocument()->renderer()->screenToTileCoords(offsetPos);
    const int x = qFloor(tilePosF.x());
    const int y = qFloor(tilePosF.y());
    setStatusInfo(QStringLiteral("%1, %2 (%3, %4)").arg(x).arg(y).arg(pixelPos.x()).arg(pixelPos.y()));
}

void AbstractWorldTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    setTargetMap(mapAt(event->scenePos()));

    if (event->button() == Qt::RightButton)
        showContextMenu(event);
}

void AbstractWorldTool::languageChanged()
{
    languageChangedImpl();
}

void AbstractWorldTool::languageChangedImpl()
{
    mNewWorldForMapAction->setText(tr("Create a new world containing the current map"));
    mAddAnotherMapToWorldAction->setText(tr("Add another map to the current world"));
    mAddMapToWorldAction->setText(tr("Add the current map to a loaded world"));
    mRemoveMapFromWorldAction->setText(tr("Remove the current map from the current world"));
}

void AbstractWorldTool::mapDocumentChanged(MapDocument *oldDocument,
                                           MapDocument *newDocument)
{
    // The enabled state of the actions depends on the map's file name
    if (oldDocument)
        disconnect(oldDocument, &Document::fileNameChanged,
                   this, &AbstractWorldTool::updateEnabledState);
    if (newDocument)
        connect(newDocument, &Document::fileNameChanged,
                this, &AbstractWorldTool::updateEnabledState);
}

void AbstractWorldTool::updateEnabledState()
{
    const bool hasWorlds = !WorldManager::instance().worlds().isEmpty();
    MapDocument *map = mapDocument();
    const auto worldDocument = worldForMap(map);

    // Maps that are not in a world can still be resized
    setEnabled(map && (!worldDocument || worldDocument->world()->canBeModified()));

    // When the map is not in a world, only the action for creating a new
    // world is shown, which guides the user to create one first
    mNewWorldForMapAction->setVisible(!worldDocument);
    mNewWorldForMapAction->setEnabled(map && !map->fileName().isEmpty() && !worldDocument);

    mAddAnotherMapToWorldAction->setVisible(worldDocument);
    mAddAnotherMapToWorldAction->setEnabled(worldDocument);

    mAddMapToWorldAction->setVisible(!worldDocument);
    mAddMapToWorldAction->setEnabled(hasWorlds && !worldDocument);

    mRemoveMapFromWorldAction->setVisible(worldDocument);
    mRemoveMapFromWorldAction->setEnabled(worldDocument);
}

MapDocument *AbstractWorldTool::mapAt(const QPointF &pos) const
{
    const QList<QGraphicsItem *> &items = mapScene()->items(pos);

    for (QGraphicsItem *item : items) {
        if (!item->isEnabled())
            continue;

        auto mapItem = qgraphicsitem_cast<MapItem*>(item);
        if (mapItem)
            return mapItem->mapDocument();
    }
    return nullptr;
}

// Finds a resize handle near the cursor on any map, hit-tested in view
// coordinates so the hit area matches the on-screen handle size at any zoom,
// returning its index and setting mapDocument to the owning map, or -1 if none
int AbstractWorldTool::resizeHandleNear(const QPointF &scenePos, MapDocument *&mapDocument) const
{
    const auto views = mapScene()->views();
    if (views.isEmpty())
        return -1;

    const QTransform viewTransform = views.first()->viewportTransform();
    const QPointF viewPos = viewTransform.map(scenePos);
    const qreal radius = Utils::dpiScaled(10.0);

    const auto items = mapScene()->items();
    for (QGraphicsItem *item : items) {
        auto mapItem = qgraphicsitem_cast<MapItem*>(item);
        if (!mapItem || !mapItem->isEnabled())
            continue;

        const auto handlePos = resizeHandlePositions(mapRect(mapItem->mapDocument()));
        for (int i = 0; i < HandleCount; ++i) {
            const QPointF delta = viewPos - viewTransform.map(handlePos[i]);
            if (qAbs(delta.x()) <= radius && qAbs(delta.y()) <= radius) {
                mapDocument = mapItem->mapDocument();
                return i;
            }
        }
    }
    return -1;
}

bool AbstractWorldTool::mapCanBeMoved(MapDocument *mapDocument) const
{
    if (!mapDocument)
        return false;
    auto worldDocument = worldForMap(mapDocument);
    return worldDocument && worldDocument->world()->canBeModified();
}

// Resizing only changes the map itself, so it also works without a world
bool AbstractWorldTool::mapCanBeResized(MapDocument *mapDocument) const
{
    if (!mapDocument)
        return false;
    auto worldDocument = worldForMap(mapDocument);
    return !worldDocument || worldDocument->world()->canBeModified();
}

QRect AbstractWorldTool::mapRect(MapDocument *mapDocument) const
{
    auto rect = mapDocument->renderer()->mapBoundingRect();

    if (auto item = mapScene()->mapItem(mapDocument))
        rect.translate(item->pos().toPoint());

    return rect;
}

WorldDocument *AbstractWorldTool::worldForMap(MapDocument *mapDocument) const
{
    if (!mapDocument)
        return nullptr;
    return WorldManager::instance().worldForMap(mapDocument->fileName()).data();
}

/**
 * Shows the context menu for adding/removing maps from worlds.
 */
void AbstractWorldTool::showContextMenu(QGraphicsSceneMouseEvent *event)
{
    QMenu menu;

    if (auto currentWorldDocument = worldForMap(mapDocument())) {
        QPoint insertPos = event->scenePos().toPoint();
        insertPos += mapRect(mapDocument()).topLeft();

        menu.addAction(QIcon(QLatin1String(":images/24/world-map-add-other.png")),
                       tr("Add a Map to World \"%2\"")
                       .arg(currentWorldDocument->displayName()),
                       this, [=] { addAnotherMapToWorld(insertPos); });

        MapDocument *targetDocument = targetMap();
        if (targetDocument != nullptr && targetDocument != mapDocument()) {
            const QString &targetFilename = targetDocument->fileName();
            menu.addAction(QIcon(QLatin1String(":images/24/world-map-remove-this.png")),
                           tr("Remove \"%1\" from World \"%2\"")
                           .arg(targetDocument->displayName(),
                                currentWorldDocument->displayName()),
                           this, [=] { removeFromWorld(currentWorldDocument, targetFilename); });
        }
    } else {
        menu.addAction(QIcon(QLatin1String(":images/24/world-map-add-other.png")),
                       tr("New World Containing \"%1\"")
                       .arg(mapDocument()->displayName()),
                       this, &AbstractWorldTool::createWorldForCurrentMap)
                ->setEnabled(!mapDocument()->fileName().isEmpty());

        populateAddToWorldMenu(menu);
    }

    menu.exec(event->screenPos());
}

void AbstractWorldTool::populateAddToWorldMenu(QMenu &menu)
{
    for (auto &worldDocument : WorldManager::instance().worlds()) {
        if (!worldDocument->world()->canBeModified())
            continue;

        auto action = menu.addAction(tr("Add \"%1\" to World \"%2\"")
                                     .arg(mapDocument()->displayName(),
                                          worldDocument->displayName()),
                                     this, [=] { addToWorld(worldDocument.data()); });
        action->setEnabled(!mapDocument()->fileName().isEmpty());
    }
}

void AbstractWorldTool::addAnotherMapToWorldAtCenter()
{
    MapView *view = DocumentManager::instance()->viewForDocument(mapDocument());
    addAnotherMapToWorld(view->viewCenter().toPoint());
}

void AbstractWorldTool::addAnotherMapToWorld(QPoint insertPos)
{
    MapDocument *map = mapDocument();
    auto worldDocument = worldForMap(map);

    if (!worldDocument)
        return;

    const QDir dir = QFileInfo(map->fileName()).dir();
    const QString lastPath = QDir::cleanPath(dir.absolutePath());
    QString filter = tr("All Files (*)");
    FormatHelper<MapFormat> helper(FileFormat::ReadWrite, filter);

    QString fileName = QFileDialog::getOpenFileName(MainWindow::instance(), tr("Load Map"), lastPath,
                                                    helper.filter());
    if (fileName.isEmpty())
        return;

    // Open the map if it's already part of one of the loaded worlds
    if (WorldManager::instance().worldForMap(fileName)) {
        DocumentManager::instance()->openFile(fileName);
        return;
    }

    QString error;
    DocumentPtr document = DocumentManager::instance()->loadDocument(fileName, nullptr, &error);

    if (!document || document->type() != Document::MapDocumentType) {
        QMessageBox::critical(MainWindow::instance(),
                              tr("Error Opening File"),
                              tr("Error opening '%1':\n%2").arg(fileName, error));
        return;
    }

    const QRect rect { snapPoint(insertPos, map), QSize(0, 0) };

    auto undoStack = worldDocument->undoStack();
    undoStack->push(new AddMapCommand(worldDocument, fileName, rect));
}

void AbstractWorldTool::createWorldForCurrentMap()
{
    if (auto map = mapDocument())
        createWorldForMap(map);
}

// Asks for a file name and creates a new world with the given map in it.
// Returns null when the map is already part of a world, the user cancels or
// the world could not be saved.
WorldDocument *AbstractWorldTool::createWorldForMap(MapDocument *map)
{
    if (map->fileName().isEmpty() || worldForMap(map))
        return nullptr;

    const QFileInfo fileInfo(map->fileName());
    const QString suggestedFileName
            = fileInfo.dir().filePath(fileInfo.completeBaseName() +
                                      QStringLiteral(".world"));

    auto worldDocument = MainWindow::instance()->createNewWorld(suggestedFileName);
    if (!worldDocument)
        return nullptr;

    const QRect rect = map->renderer()->mapBoundingRect();
    worldDocument->undoStack()->push(new AddMapCommand(worldDocument, map->fileName(), rect));

    return worldDocument;
}

void AbstractWorldTool::removeCurrentMapFromWorld()
{
    if (auto currentWorldDocument = worldForMap(mapDocument()))
        removeFromWorld(currentWorldDocument, mapDocument()->fileName());
}

void AbstractWorldTool::removeFromWorld(WorldDocument *worldDocument,
                                        const QString &mapFileName)
{
    if (mapFileName.isEmpty())
        return;

    QUndoStack *undoStack = worldDocument->undoStack();
    undoStack->push(new RemoveMapCommand(worldDocument, mapFileName));
}

void AbstractWorldTool::addToWorld(WorldDocument *worldDocument)
{
    MapDocument *document = mapDocument();
    if (document->fileName().isEmpty())
        return;

    QRect rect = document->renderer()->mapBoundingRect();

    // Position the map alongside the last map by default
    const World *world = worldDocument->world();
    if (!world->maps.isEmpty()) {
        const QRect &lastWorldRect = world->maps.last().rect;
        rect.moveTo(lastWorldRect.right() + 1, lastWorldRect.top());
    }

    QUndoStack *undoStack = worldDocument->undoStack();
    undoStack->push(new AddMapCommand(worldDocument, document->fileName(), rect));
}

QUndoStack *AbstractWorldTool::undoStack()
{
    auto worldDocument = worldForMap(mapDocument());
    return worldDocument ? worldDocument->undoStack() : nullptr;
}

void AbstractWorldTool::populateToolBar(QToolBar *toolBar)
{
    toolBar->addAction(mNewWorldForMapAction);
    toolBar->addAction(mAddAnotherMapToWorldAction);
    toolBar->addAction(mAddMapToWorldAction);
    toolBar->addAction(mRemoveMapFromWorldAction);

    auto addMapToWorldButton = qobject_cast<QToolButton*>(toolBar->widgetForAction(mAddMapToWorldAction));
    auto addToWorldMenu = new QMenu(addMapToWorldButton);

    connect(addToWorldMenu, &QMenu::aboutToShow, this, [=] {
        addToWorldMenu->clear();
        populateAddToWorldMenu(*addToWorldMenu);
    });

    addMapToWorldButton->setPopupMode(QToolButton::InstantPopup);
    addMapToWorldButton->setMenu(addToWorldMenu);

    // Workaround to make the shortcut for opening the menu work
    connect(mAddMapToWorldAction, &QAction::triggered, addMapToWorldButton, [=] {
        addMapToWorldButton->setDown(true);
        addMapToWorldButton->showMenu();
    });
}

QSize AbstractWorldTool::snapSize(MapDocument *document) const
{
    // Snap to the world grid when set and enabled, otherwise the tile size
    if (Preferences::instance()->snapToWorldGrid()) {
        if (auto worldDocument = worldForMap(document)) {
            const QSize gridSize = worldDocument->world()->gridSize;
            if (!gridSize.isEmpty())
                return gridSize;
        }
    }

    return document->map()->tileSize();
}

QPoint AbstractWorldTool::snapPoint(QPoint point, MapDocument *document) const
{
    const QSize size = snapSize(document);
    point.setX(qRound(qreal(point.x()) / size.width()) * size.width());
    point.setY(qRound(qreal(point.y()) / size.height()) * size.height());
    return point;
}

void AbstractWorldTool::setTargetMap(MapDocument *mapDocument)
{
    mTargetMap = mapDocument;
    updateSelectionRectangle();
}

void AbstractWorldTool::updateSelectionRectangle()
{
    if (mTargetMap) {
        setSelectionScreenRect(mapRect(mTargetMap));
    } else {
        mSelectionRectangle->setVisible(false);
        for (auto &handle : mResizeHandles)
            handle->setVisible(false);
    }
}

void AbstractWorldTool::setSelectionScreenRect(const QRect &rect)
{
    mSelectionRectangle->setRectangle(rect);
    mSelectionRectangle->setVisible(true);

    // Place the eight handles on the corners and edge midpoints
    const auto handlePos = resizeHandlePositions(rect);
    for (int i = 0; i < HandleCount; ++i) {
        mResizeHandles[i]->setPos(handlePos[i]);
        mResizeHandles[i]->setVisible(true);
    }
}

// Move the camera back by offset, to keep the active map steady after it shifts
void AbstractWorldTool::recenterView(const QPoint &offset)
{
    if (offset.isNull())
        return;

    MapView *view = DocumentManager::instance()->viewForDocument(mapDocument());
    view->forceCenterOn(view->viewCenter() - offset);
}

} // namespace Tiled

#include "moc_abstractworldtool.cpp"
