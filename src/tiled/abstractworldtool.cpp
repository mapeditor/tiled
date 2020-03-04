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

#include "preferences.h"
#include "documentmanager.h"
#include "mapdocument.h"
#include "map.h"
#include "mapscene.h"
#include "mapeditor.h"
#include "maprenderer.h"
#include "tile.h"
#include "utils.h"
#include "worlddocument.h"
#include "worldmanager.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
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
{
    AbstractWorldTool::languageChanged();

    WorldManager &worldManager = WorldManager::instance();
    connect(&worldManager, &WorldManager::worldsChanged, this, &AbstractWorldTool::updateEnabledState);
}

void AbstractWorldTool::activate(MapScene *scene)
{
    mMapScene = scene;
}

void AbstractWorldTool::deactivate(MapScene *)
{
    mMapScene = nullptr;
}

void AbstractWorldTool::mouseLeft()
{
    setStatusInfo(QString());
}

void AbstractWorldTool::mouseMoved(const QPointF &pos,
                                   Qt::KeyboardModifiers)
{
    // Update target map
    mTargetMap = mapAt(pos);

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
    if (event->button() == Qt::RightButton)
        showContextMenu(event);
}

void AbstractWorldTool::languageChanged()
{
}

void AbstractWorldTool::updateEnabledState()
{
    bool hasWorlds = !WorldManager::instance().worlds().isEmpty();
    const World *world = constWorld(mapDocument());
    setEnabled(mapDocument() && hasWorlds && (world == nullptr || world->canBeModified()));
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

        QAction *addToWorldAction = menu.addAction(tr("Add a Map to World %2")
                                                   .arg(currentWorld->displayName()),
                                                   this, [=] {
            addAnotherMapToWorld(insertPos);
        });
        addToWorldAction->setIcon(QIcon(QLatin1String(":/images/16/add.png")));

        if (targetDocument != nullptr && targetDocument != currentDocument) {
            QAction *removeFromWorldAction = menu.addAction(tr("Remove %1 from World %2")
                                                            .arg(targetDocument->displayName())
                                                            .arg(targetWorld->displayName()));
            removeFromWorldAction->setIcon(QIcon(QLatin1String(":/images/16/remove.png")));
            const QString targetFilename = targetDocument->fileName();
            connect(removeFromWorldAction, &QAction::triggered, this, [=] {
                removeFromWorld(targetFilename);
            });
        }
    } else {
        WorldManager &manager = WorldManager::instance();
        const QStringList worlds = manager.loadedWorldFiles();
        for (const QString &worldFilename : worlds) {
            if (!manager.worlds()[worldFilename]->canBeModified())
                continue;
            QAction *addToWorldAction = menu.addAction(tr("Add %1 to World %2")
                                                       .arg(currentDocument->displayName())
                                                       .arg(World::displayName(worldFilename)));
            connect(addToWorldAction, &QAction::triggered, this, [=] {
                addToWorld(worldFilename);
            });
        }
    }

    menu.exec(screenPos);
}

void AbstractWorldTool::addAnotherMapToWorld(QPoint insertPos)
{
    MapDocument *map = mapDocument();
    const World *world = constWorld(map);
    if (!world)
        return;

    const QDir dir = QFileInfo(map->fileName()).dir();
    const QString lastPath = QDir::cleanPath(dir.absolutePath());
    QString filter = tr("All Files (*);;");
    QString mapFilesFilter = tr("Map Files (*.tmx)");
    filter.append(mapFilesFilter);

    auto mapEditor = static_cast<MapEditor*>(DocumentManager::instance()->editor(Document::DocumentType::MapDocumentType));
    QString mapFile = QFileDialog::getOpenFileName(mapEditor->editorWidget(), tr("Load Map"), lastPath,
                                                   filter, &mapFilesFilter);
    if (mapFile.isEmpty())
        return;

    const World *constWorldForSelectedMap =  WorldManager::instance().worldForMap(mapFile);
    if (constWorldForSelectedMap) {
        DocumentManager::instance()->openFile(mapFile);
        return;
    }

    const QRect rect = QRect(snapPoint(insertPos, map), QSize(0, 0));

    undoStack()->push(new AddMapCommand(world->fileName, mapFile, rect));
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

QPoint AbstractWorldTool::snapPoint(QPoint point, MapDocument *document) const
{
    point.setX(point.x() - point.x() % document->map()->tileWidth());
    point.setY(point.y() - point.y() % document->map()->tileHeight());
    return point;
}

} // namespace Tiled
