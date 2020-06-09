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
#include "documentmanager.h"
#include "mainwindow.h"
#include "map.h"
#include "mapdocument.h"
#include "mapeditor.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "selectionrectangle.h"
#include "tile.h"
#include "utils.h"
#include "worlddocument.h"
#include "worldmanager.h"

#include <QAction>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>
#include <QtMath>

#include "qtcompat_p.h"

namespace Tiled {

class AddMapCommand : public QUndoCommand
{
public:
    AddMapCommand(const QString &worldName, const QString &mapName, const QRect &rect)
        : mWorldName(worldName)
        , mMapName(mapName)
        , mRect(rect)
    {
    }

    void undo() override
    {
        WorldManager::instance().removeMap(mMapName);
    }

    void redo() override
    {
        WorldManager::instance().addMap(mWorldName, mMapName, mRect);
    }

private:
    QString mWorldName;
    QString mMapName;
    QRect mRect;
};

class RemoveMapCommand : public QUndoCommand
{
public:
    RemoveMapCommand(const QString &mapName)
        : mMapName(mapName)
    {
        const WorldManager &manager = WorldManager::instance();
        const World *world = manager.worldForMap(mMapName);
        mPreviousRect = world->mapRect(mMapName);
        mWorldName = world->fileName;
    }

    void undo() override
    {
        WorldManager::instance().addMap(mWorldName, mMapName, mPreviousRect);
    }

    void redo() override
    {
        // ensure we're switching to a differnet map in case the current map is removed
        DocumentManager *manager = DocumentManager::instance();
        if (manager->currentDocument() && manager->currentDocument()->fileName() == mMapName) {
            const World *world = WorldManager::instance().worldForMap(mMapName);
            for (World::MapEntry &entry : world->allMaps())
                if (entry.fileName != mMapName) {
                    manager->switchToDocument(entry.fileName);
                    break;
                }
        }
        WorldManager::instance().removeMap(mMapName);
    }

private:
    QString mWorldName;
    QString mMapName;
    QRect mPreviousRect;
};


AbstractWorldTool::AbstractWorldTool(Id id,
                                     const QString &name,
                                     const QIcon &icon,
                                     const QKeySequence &shortcut,
                                     QObject *parent)
    : AbstractTool(id, name, icon, shortcut, parent)
    , mSelectionRectangle(new SelectionRectangle)
{
    mSelectionRectangle->setVisible(false);

    WorldManager &worldManager = WorldManager::instance();
    connect(&worldManager, &WorldManager::worldsChanged, this, &AbstractWorldTool::updateEnabledState);

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
}

AbstractWorldTool::~AbstractWorldTool() = default;

void AbstractWorldTool::activate(MapScene *scene)
{
    scene->addItem(mSelectionRectangle.get());
    connect(scene, &MapScene::sceneRefreshed, this, &AbstractWorldTool::updateSelectionRectangle);
    mMapScene = scene;
}

void AbstractWorldTool::deactivate(MapScene *scene)
{
    scene->removeItem(mSelectionRectangle.get());
    disconnect(scene, &MapScene::sceneRefreshed, this, &AbstractWorldTool::updateSelectionRectangle);
    mMapScene = nullptr;
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
        offsetPos -= layer->totalOffset();

    const QPoint pixelPos = offsetPos.toPoint();
    const QPointF tilePosF = mapDocument()->renderer()->screenToTileCoords(offsetPos);
    const int x = qFloor(tilePosF.x());
    const int y = qFloor(tilePosF.y());
    setStatusInfo(QString(QLatin1String("%1, %2 (%3, %4)")).arg(x).arg(y).arg(pixelPos.x()).arg(pixelPos.y()));
}

void AbstractWorldTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    setTargetMap(mapAt(event->scenePos()));

    if (event->button() == Qt::RightButton)
        showContextMenu(event);
}

void AbstractWorldTool::languageChanged()
{
    mAddAnotherMapToWorldAction->setText(tr("Add another map to the current world"));
    mAddMapToWorldAction->setText(tr("Add the current map to a loaded world"));
    mRemoveMapFromWorldAction->setText(tr("Remove the current map from the current world"));
}

void AbstractWorldTool::updateEnabledState()
{
    const bool hasWorlds = !WorldManager::instance().worlds().isEmpty();
    const World *world = constWorld(mapDocument());
    setEnabled(mapDocument() && hasWorlds && (world == nullptr || world->canBeModified()));

    // update toolbar actions
    mAddMapToWorldAction->setEnabled(hasWorlds && world == nullptr);
    mRemoveMapFromWorldAction->setEnabled(world != nullptr);
    mAddAnotherMapToWorldAction->setEnabled(world != nullptr);
}

MapDocument *AbstractWorldTool::mapAt(const QPointF &pos) const
{
    const QList<QGraphicsItem *> &items = mMapScene->items(pos);

    for (QGraphicsItem *item : items) {
        if (!item->isEnabled())
            continue;

        auto mapItem = qgraphicsitem_cast<MapItem*>(item);
        if (mapItem)
            return mapItem->mapDocument();
    }
    return nullptr;
}

bool AbstractWorldTool::mapCanBeMoved(MapDocument *mapDocument) const
{
    if (!mapDocument)
        return false;
    const World *world = constWorld(mapDocument);
    return world != nullptr && world->canBeModified();
}


QRect AbstractWorldTool::mapRect(MapDocument *mapDocument) const
{
    QRect rect = mapDocument->renderer()->mapBoundingRect();
    if (const World *world = constWorld(mapDocument))
        rect.translate(world->mapRect(mapDocument->fileName()).topLeft());
    return rect;
}

const World *AbstractWorldTool::constWorld(MapDocument *mapDocument) const
{
    if (!mapDocument)
        return nullptr;
    return WorldManager::instance().worldForMap(mapDocument->fileName());
}

/**
 * Shows the context menu for adding/removing maps from worlds.
 */
void AbstractWorldTool::showContextMenu(QGraphicsSceneMouseEvent *event)
{
    MapDocument *currentDocument = mapDocument();
    MapDocument *targetDocument = targetMap();
    const World *currentWorld = constWorld(currentDocument);
    const World *targetWorld = constWorld(targetDocument);

    const auto screenPos = event->screenPos();

    QMenu menu;
    if (currentWorld) {
        QPoint insertPos = event->scenePos().toPoint();
        insertPos += mapRect(currentDocument).topLeft();

        menu.addAction(QIcon(QLatin1String(":images/24/world-map-add-other.png")),
                       tr("Add a Map to World \"%2\"")
                       .arg(currentWorld->displayName()),
                       this, [=] { addAnotherMapToWorld(insertPos); });

        if (targetDocument != nullptr && targetDocument != currentDocument) {
            const QString targetFilename = targetDocument->fileName();
            menu.addAction(QIcon(QLatin1String(":images/24/world-map-remove-this.png")),
                           tr("Remove \"%1\" from World \"%2\"")
                           .arg(targetDocument->displayName())
                           .arg(targetWorld->displayName()),
                           this, [=] { removeFromWorld(targetFilename); });
        }
    } else {
        for (const World *world : WorldManager::instance().worlds()) {
            if (!world->canBeModified())
                continue;

            menu.addAction(tr("Add \"%1\" to World \"%2\"")
                           .arg(currentDocument->displayName())
                           .arg(world->displayName()),
                           this, [this, fileName = world->fileName] { addToWorld(fileName); });
        }
    }

    menu.exec(screenPos);
}

void AbstractWorldTool::addAnotherMapToWorldAtCenter()
{
    DocumentManager *manager = DocumentManager::instance();
    MapView *view = manager->viewForDocument(mapDocument());
    const QRectF viewRect { view->viewport()->rect() };
    const QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);
    addAnotherMapToWorld(sceneViewRect.center().toPoint());
}

void AbstractWorldTool::addAnotherMapToWorld(QPoint insertPos)
{
    MapDocument *map = mapDocument();
    const World *world = constWorld(map);
    if (!world)
        return;

    const QDir dir = QFileInfo(map->fileName()).dir();
    const QString lastPath = QDir::cleanPath(dir.absolutePath());
    QString filter = tr("All Files (*)");
    FormatHelper<MapFormat> helper(FileFormat::ReadWrite, filter);

    QString fileName = QFileDialog::getOpenFileName(MainWindow::instance(), tr("Load Map"), lastPath,
                                                    helper.filter());
    if (fileName.isEmpty())
        return;

    const World *constWorldForSelectedMap = WorldManager::instance().worldForMap(fileName);
    if (constWorldForSelectedMap) {
        DocumentManager::instance()->openFile(fileName);
        return;
    }

    QString error;
    DocumentPtr document = DocumentManager::instance()->loadDocument(fileName, nullptr, &error);

    if (!document) {
        QMessageBox::critical(MainWindow::instance(),
                              tr("Error Opening File"),
                              tr("Error opening '%1':\n%2").arg(fileName, error));
        return;
    }

    const QRect rect { snapPoint(insertPos, map), QSize(0, 0) };

    undoStack()->push(new AddMapCommand(world->fileName, fileName, rect));
}

void AbstractWorldTool::removeCurrentMapFromWorld()
{
    removeFromWorld(mapDocument()->fileName());
}

void AbstractWorldTool::removeFromWorld(const QString &mapFileName)
{
    undoStack()->push(new RemoveMapCommand(mapFileName));
}

void AbstractWorldTool::addToWorld(const QString &worldFileName)
{
    MapDocument *document = mapDocument();
    QSize size = document->map()->size();
    size.setWidth(size.width() * document->map()->tileWidth());
    size.setHeight(size.height() * document->map()->tileHeight());
    const QRect rect = QRect(QPoint(0, 0), size);
    QUndoStack *undoStack = DocumentManager::instance()->ensureWorldDocument(worldFileName)->undoStack();
    undoStack->push(new AddMapCommand(worldFileName, document->fileName(), rect));
}

QUndoStack *AbstractWorldTool::undoStack()
{
    const World *world = constWorld(mapDocument());
    if (!world)
        return nullptr;
    return DocumentManager::instance()->ensureWorldDocument(world->fileName)->undoStack();
}

void AbstractWorldTool::populateToolBar(QToolBar *toolBar)
{
    toolBar->addAction(mAddAnotherMapToWorldAction);
    toolBar->addAction(mAddMapToWorldAction);
    toolBar->addAction(mRemoveMapFromWorldAction);

    auto addMapToWorldButton = qobject_cast<QToolButton*>(toolBar->widgetForAction(mAddMapToWorldAction));
    auto addToWorldMenu = new QMenu(addMapToWorldButton);

    connect(addToWorldMenu, &QMenu::aboutToShow, [=] {
        addToWorldMenu->clear();

        for (const World *world : WorldManager::instance().worlds()) {
            if (!world->canBeModified())
                continue;

            addToWorldMenu->addAction(tr("Add \"%1\" to World \"%2\"")
                                      .arg(mapDocument()->displayName())
                                      .arg(world->displayName()),
                                      this, [this, fileName = world->fileName] { addToWorld(fileName); });
        }
    });

    addMapToWorldButton->setPopupMode(QToolButton::InstantPopup);
    addMapToWorldButton->setMenu(addToWorldMenu);

    // Workaround to make the shortcut for opening the menu work
    connect(mAddMapToWorldAction, &QAction::triggered, addMapToWorldButton, [=] {
        addMapToWorldButton->setDown(true);
        addMapToWorldButton->showMenu();
    });
}

QPoint AbstractWorldTool::snapPoint(QPoint point, MapDocument *document) const
{
    point.setX(point.x() - point.x() % document->map()->tileWidth());
    point.setY(point.y() - point.y() % document->map()->tileHeight());
    return point;
}

void AbstractWorldTool::setTargetMap(MapDocument *mapDocument)
{
    mTargetMap = mapDocument;
    updateSelectionRectangle();
}

void AbstractWorldTool::updateSelectionRectangle()
{
    if (auto item = mMapScene->mapItem(mTargetMap)) {
        auto rect = mapRect(mTargetMap);
        rect.moveTo(item->pos().toPoint());

        mSelectionRectangle->setVisible(true);
        mSelectionRectangle->setRectangle(rect);
    } else {
        mSelectionRectangle->setVisible(false);
    }
}

} // namespace Tiled
