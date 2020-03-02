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

using namespace Tiled;

namespace Tiled
{

class AddMapCommand : public QUndoCommand
{
public:
    AddMapCommand(const QString& worldName, const QString& mapName, const QRect& rect) {
        mMapName = mapName;
        mWorldName = worldName;
        mRect = rect;
    }

    void undo() override {
        WorldManager& manager = WorldManager::instance();
        manager.removeMap( mMapName );
    }

    void redo() override {
        WorldManager& manager = WorldManager::instance();
        manager.addMap( mWorldName, mMapName, mRect );
    }

private:
    QString mWorldName;
    QString mMapName;
    QRect mRect;
};

class RemoveMapCommand : public QUndoCommand
{
public:
    RemoveMapCommand(const QString& mapName) {
        mMapName = mapName;
        WorldManager& manager = WorldManager::instance();
        const World* world = manager.worldForMap(mMapName);
        mPreviousRect = world->mapRect(mMapName);
        mWorldName = world->fileName;
    }

    void undo() override {
        WorldManager& manager = WorldManager::instance();
        manager.addMap( mWorldName, mMapName, mPreviousRect );
    }

    void redo() override {
        WorldManager& manager = WorldManager::instance();
        manager.removeMap( mMapName );
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
    , mMapScene(nullptr)
    , mTargetMap(nullptr)
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
    // Take into account the offset of the current layer
    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer())
        offsetPos -= layer->totalOffset();

    const QPoint pixelPos = offsetPos.toPoint();

    MapDocument *document = targetMap();
    if (!document) {
        return;
    }

    const QPointF tilePosF = document->renderer()->screenToTileCoords(offsetPos);
    const int x = qFloor(tilePosF.x());
    const int y = qFloor(tilePosF.y());
    setStatusInfo(QString(QLatin1String("%1, %2 (%3, %4)")).arg(x).arg(y).arg(pixelPos.x()).arg(pixelPos.y()));
}

void AbstractWorldTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->screenPos());
    }
}

void AbstractWorldTool::languageChanged()
{

}


void AbstractWorldTool::updateEnabledState()
{
    setEnabled(currentMap() != nullptr && WorldManager::instance().loadedWorldFiles().count() > 0);
}

bool AbstractWorldTool::targetMapCanBeMoved() const
{
    if(!targetMap()) {
        return false;
    }
    return WorldManager::instance().mapCanBeModified( targetMap()->fileName() );
}

QRect AbstractWorldTool::targetMapRect() const
{
    const World* pWorld = targetConstWorld();
    QRect rect = pWorld->mapRect( targetMap()->fileName() );
    QSize size = targetMap()->map()->size();
    size.setWidth(size.width() * targetMap()->map()->tileWidth());
    size.setHeight(size.height() * targetMap()->map()->tileHeight());
    return QRect(rect.topLeft(), size);
}

QRect AbstractWorldTool::currentMapRect() const
{
    const World* pWorld = targetConstWorld();
    QRect rect = pWorld->mapRect( mapDocument()->fileName() );
    QSize size = currentMap()->map()->size();
    size.setWidth(size.width() * currentMap()->map()->tileWidth());
    size.setHeight(size.height() * currentMap()->map()->tileHeight());
    return QRect(rect.topLeft(), size);
}

const World *AbstractWorldTool::targetConstWorld() const
{
    if(!targetMap()) {
        return nullptr;
    }
    return WorldManager::instance().worldForMap( targetMap()->fileName() );
}

const World *AbstractWorldTool::currentConstWorld() const
{
    if (!currentMap()) {
        return nullptr;
    }
    return WorldManager::instance().worldForMap(currentMap()->fileName());
}

/**
 * Shows the context menu for map objects. The menu allows you to duplicate and
 * remove the map objects, or to edit their properties.
 */
void AbstractWorldTool::showContextMenu(QPoint screenPos)
{
    MapDocument *currentDocument = currentMap();
    MapDocument *targetDocument = targetMap();
    const World *currentWorld = currentConstWorld();
    const World *targetWorld = targetConstWorld();

    mMousePos = currentDocument->renderer()->screenToPixelCoords(screenPos).toPoint();

    QMenu menu;
    if (currentWorld) {
        QAction *addToWorldAction = menu.addAction(tr("Add another map to world %2")
                                                   .arg(currentWorld->displayName()),
                                                  this, &AbstractWorldTool::addAnotherMapToWorld, QKeySequence(tr("A") ) );
        addToWorldAction->setIcon(QIcon(QLatin1String(":/images/16/add.png")));

        if (targetDocument != nullptr && targetDocument != currentDocument) {
            QAction *removeFromWorldAction = menu.addAction(tr("remove %1 from world %2")
                                                            .arg(targetDocument->displayName())
                                                            .arg(targetWorld->displayName()));
            removeFromWorldAction->setIcon(QIcon(QLatin1String(":/images/16/remove.png")));
            QString targetFilename = targetDocument->fileName();
            connect(removeFromWorldAction, &QAction::triggered, this, [this,targetFilename] {
                removeFromWorld(targetFilename);
            });
        }
    } else {
        QStringList worlds = WorldManager::instance().loadedWorldFiles();
        for (QString& worldFilename : worlds) {
            QAction *addToWorldAction = menu.addAction(tr("Add %1 to world %2")
                                                       .arg(currentMap()->displayName())
                                                       .arg(World::displayName(worldFilename)));
            connect(addToWorldAction, &QAction::triggered, this, [this,worldFilename] {
                addToWorld(worldFilename);
            });
        }
    }

    QAction *action = menu.exec(screenPos);
    if (!action)
        return;
}

void AbstractWorldTool::addAnotherMapToWorld()
{
    MapDocument *map = currentMap();
    const World* constWorld = currentConstWorld();
    if(!constWorld) {
        return;
    }

    QDir dir = QFileInfo(map->fileName()).dir();

    QString lastPath = QDir::cleanPath(dir.absolutePath());
    QString filter = tr("All Files (*);;");
    QString mapFilesFilter = tr("Map Files (*.tmx)");
    filter.append(mapFilesFilter);

    MapEditor* mapEditor = static_cast<MapEditor*>(DocumentManager::instance()->editor(Document::DocumentType::MapDocumentType));
    QString mapFile = QFileDialog::getOpenFileName(mapEditor->editorWidget(), tr("Load Map"), lastPath,
                                                     filter, &mapFilesFilter);
    if (mapFile.isEmpty())
        return;

    const World* constWorldForSelectedMap =  WorldManager::instance().worldForMap(mapFile);
    if (constWorldForSelectedMap) {
        DocumentManager::instance()->openFile(mapFile);
        return;
    }

    QSize size(0,0);
    QRect rect = QRect(snapPoint(mMousePos, map), size);

    undoStack()->push(new AddMapCommand(constWorld->fileName, mapFile, rect));
}

void AbstractWorldTool::removeFromWorld(const QString& mapFileName)
{
    undoStack()->push(new RemoveMapCommand(mapFileName));
}

void AbstractWorldTool::addToWorld(const QString& worldFileName)
{
    MapDocument *document = currentMap();
    QSize size = document->map()->size();
    size.setWidth(size.width() * document->map()->tileWidth());
    size.setHeight(size.height() * document->map()->tileHeight());
    QRect rect = QRect( QPoint(0,0), size);
    QUndoStack* undoStack = DocumentManager::instance()->ensureWorldDocument(worldFileName)->undoStack();
    undoStack->push(new AddMapCommand(worldFileName, document->fileName(), rect));
}

QUndoStack *AbstractWorldTool::undoStack()
{
    const World *world = currentConstWorld();
    if(world) {
        return DocumentManager::instance()->ensureWorldDocument(world->fileName)->undoStack();
    }
    return nullptr;
}

void AbstractWorldTool::setTargetMap(MapDocument *document)
{
    if (mTargetMap != document) {
        mTargetMap = document;
    }
}

MapDocument *AbstractWorldTool::targetMap() const
{
    return mTargetMap;// != nullptr ? mTargetMap : mapDocument();
}

MapDocument *AbstractWorldTool::currentMap() const
{
    return mapDocument();
}

QPoint AbstractWorldTool::snapPoint(QPoint point, MapDocument *document) const
{
    point.setX(point.x() - (point.x()) % document->map()->tileWidth());
    point.setY(point.y() - (point.y()) % document->map()->tileHeight());
    return point;
}

}
