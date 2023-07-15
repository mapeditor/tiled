/*
 * abstracttiletool.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "abstracttiletool.h"

#include "brushitem.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "tilelayer.h"
#include "tilestamp.h"

#include <QKeyEvent>
#include <QtMath>

using namespace Tiled;

AbstractTileTool::AbstractTileTool(Id id,
                                   const QString &name,
                                   const QIcon &icon,
                                   const QKeySequence &shortcut,
                                   BrushItem *brushItem,
                                   QObject *parent)
    : AbstractTool(id, name, icon, shortcut, parent)
    , mTilePositionMethod(OnTiles)
    , mBrushItem(brushItem)
    , mBrushVisible(false)
{
    setTargetLayerType(Layer::TileLayerType);

    if (!mBrushItem)
        mBrushItem = new BrushItem;
    mBrushItem->setVisible(false);
    mBrushItem->setZValue(10000);
}

AbstractTileTool::~AbstractTileTool()
{
    delete mBrushItem;
}

void AbstractTileTool::activate(MapScene *scene)
{
    scene->addItem(mBrushItem);
    AbstractTool::activate(scene);
}

void AbstractTileTool::deactivate(MapScene *scene)
{
    scene->removeItem(mBrushItem);
    AbstractTool::deactivate(scene);
}

void AbstractTileTool::mouseEntered()
{
    setBrushVisible(true);
}

void AbstractTileTool::mouseLeft()
{
    setBrushVisible(false);
}

void AbstractTileTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers)
{
    // Take into account the offset of the current layer
    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer()) {
        QPointF layerOffset = mapScene()->absolutePositionForLayer(*layer);
        offsetPos -= layerOffset;
        mBrushItem->setLayerOffset(layerOffset);
    }

    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF tilePosF = renderer->screenToTileCoords(offsetPos);
    QPoint tilePos;

    if (mTilePositionMethod == BetweenTiles)
        tilePos = tilePosF.toPoint();
    else
        tilePos = QPoint(qFloor(tilePosF.x()),
                         qFloor(tilePosF.y()));

    if (mTilePosition != tilePos) {
        mTilePosition = tilePos;

        tilePositionChanged(tilePos);
        updateStatusInfo();
    }
}

void AbstractTileTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && event->modifiers() & Qt::ControlModifier) {
        const QPointF mousePos = event->pos();
        const MapRenderer *renderer = mapDocument()->renderer();

        QList<Layer*> layers;

        const bool append = event->modifiers() & Qt::ShiftModifier;
        const bool selectAll = event->modifiers() & Qt::AltModifier;

        if (append)
            layers = mapDocument()->selectedLayers();

        LayerIterator it(mapDocument()->map(), Layer::TileLayerType);
        it.toBack();
        while (auto tileLayer = static_cast<TileLayer*>(it.previous())) {
            if (tileLayer->isHidden())
                continue;

            const QPointF layerOffset = mapScene()->absolutePositionForLayer(*tileLayer);
            const QPointF tilePosF = renderer->screenToTileCoords(mousePos - layerOffset);
            const QPoint tilePos = QPoint(qFloor(tilePosF.x()),
                                          qFloor(tilePosF.y()));

            if (!tileLayer->cellAt(tilePos - tileLayer->position()).isEmpty()) {
                if (!layers.contains(tileLayer))
                    layers.append(tileLayer);
                else if (append)
                    layers.removeOne(tileLayer);

                if (!selectAll)
                    break;
            }
        }

        if (!layers.isEmpty())
            mapDocument()->switchSelectedLayers(layers);

        return;
    }

    event->ignore();
}

void AbstractTileTool::mapDocumentChanged(MapDocument *oldDocument,
                                          MapDocument *newDocument)
{
    Q_UNUSED(oldDocument)
    mBrushItem->setMapDocument(newDocument);
}

void AbstractTileTool::updateEnabledState()
{
    AbstractTool::updateEnabledState();
    updateBrushVisibility();
}

void AbstractTileTool::updateStatusInfo()
{
    if (mBrushVisible) {
        Cell cell;
        bool hex = false;

        if (const TileLayer *tileLayer = currentTileLayer()) {
            const QPoint pos = tilePosition() - tileLayer->position();
            cell = tileLayer->cellAt(pos);
            hex = mapDocument()->renderer()->cellType() == MapRenderer::HexagonalCells;
        }

        QString tileIdString = cell.tileId() >= 0 ? QString::number(cell.tileId()) : tr("empty");

        QStringList flippedBits;
        if (cell.flippedHorizontally())
            flippedBits.append(QStringLiteral("H"));
        if (cell.flippedVertically())
            flippedBits.append(QStringLiteral("V"));
        if (cell.flippedAntiDiagonally())
            flippedBits.append(hex ? QStringLiteral("Rot60") : QStringLiteral("D"));
        if (cell.rotatedHexagonal120())
            flippedBits.append(QStringLiteral("Rot120"));

        if (!flippedBits.isEmpty()) {
            tileIdString.append(QLatin1Char(' '));
            tileIdString.append(flippedBits.join(QLatin1Char(',')));
        }

        setStatusInfo(QStringLiteral("%1, %2 [%3]")
                      .arg(mTilePosition.x())
                      .arg(mTilePosition.y())
                      .arg(tileIdString));
    } else {
        setStatusInfo(QString());
    }
}

TileLayer *AbstractTileTool::currentTileLayer() const
{
    if (mapDocument())
        if (auto currentLayer = mapDocument()->currentLayer())
            return currentLayer->asTileLayer();
    return nullptr;
}

void AbstractTileTool::updateBrushVisibility()
{
    if (!mBrushItem)
        return;

    // Show the tile brush only when at least one target layer is visible
    bool showBrush = false;
    if (mBrushVisible) {
        const auto layers = targetLayers();
        for (auto layer : layers) {
            if (!layer || !layer->isHidden()) {
                showBrush = true;
                break;
            }
        }
    }

    mBrushItem->setVisible(showBrush);
}

/**
 * Returns the target layers. The preview is automatically hidden when none of
 * the target layers are visible.
 *
 * The result may include a nullptr, which indicates a layer is about to be
 * created (which is assumed to be visible).
 */
QList<Layer *> AbstractTileTool::targetLayers() const
{
    // By default, only a current tile layer is considered the target
    QList<Layer *> layers;
    if (Layer *layer = currentTileLayer())
        layers.append(layer);
    return layers;
}

/**
 * A helper method that returns the possible target layers of a given \a stamp.
 */
QList<Layer *> AbstractTileTool::targetLayersForStamp(const TileStamp &stamp) const
{
    QList<Layer*> layers;

    if (!mapDocument())
        return layers;

    QList<const TileLayer *> sourceLayers;

    for (const TileStampVariation &variation : stamp.variations()) {
        for (const Layer *layer : variation.map->tileLayers())
            sourceLayers.append(static_cast<const TileLayer*>(layer));

        const auto targetLayers = mapDocument()->findTargetLayers(sourceLayers);
        for (TileLayer *target : targetLayers)
            if (!layers.contains(target))
                layers.append(target);

        sourceLayers.clear();
    }

    return layers;
}

void AbstractTileTool::setBrushVisible(bool visible)
{
    if (mBrushVisible == visible)
        return;

    mBrushVisible = visible;
    updateStatusInfo();
    updateBrushVisibility();
}

#include "moc_abstracttiletool.cpp"
