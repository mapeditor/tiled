/*
 * changemapproperty.cpp
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
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

#include "changemapproperty.h"

#include "changeevents.h"
#include "map.h"
#include "mapdocument.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::Property property,
                                     int value)
    : mMapDocument(mapDocument)
    , mProperty(property)
    , mIntValue(value)
{
    switch (property) {
    case Map::TileWidthProperty:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Tile Width"));
        break;
    case Map::TileHeightProperty:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Tile Height"));
        break;
    case Map::InfiniteProperty:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Infinite Property"));
        break;
    case Map::HexSideLengthProperty:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Hex Side Length"));
        break;
    case Map::CompressionLevelProperty:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Compression Level"));
        break;
    default:
        break;
    }
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     const QColor &backgroundColor)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Background Color"))
    , mMapDocument(mapDocument)
    , mProperty(Map::BackgroundColorProperty)
    , mBackgroundColor(backgroundColor)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     QSize chunkSize)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Chunk Size"))
    , mMapDocument(mapDocument)
    , mProperty(Map::ChunkSizeProperty)
    , mChunkSize(chunkSize)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::StaggerAxis staggerAxis)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Stagger Axis"))
    , mMapDocument(mapDocument)
    , mProperty(Map::StaggerAxisProperty)
    , mStaggerAxis(staggerAxis)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::StaggerIndex staggerIndex)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Stagger Index"))
    , mMapDocument(mapDocument)
    , mProperty(Map::StaggerIndexProperty)
    , mStaggerIndex(staggerIndex)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     const QPointF &parallaxOrigin)
        : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                                   "Change Parallax Origin"))
        , mMapDocument(mapDocument)
        , mProperty(Map::ParallaxOriginProperty)
        , mParallaxOrigin(parallaxOrigin)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::Orientation orientation)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Orientation"))
    , mMapDocument(mapDocument)
    , mProperty(Map::OrientationProperty)
    , mOrientation(orientation)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::RenderOrder renderOrder)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Render Order"))
    , mMapDocument(mapDocument)
    , mProperty(Map::RenderOrderProperty)
    , mRenderOrder(renderOrder)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::LayerDataFormat layerDataFormat)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Layer Data Format"))
    , mMapDocument(mapDocument)
    , mProperty(Map::LayerDataFormatProperty)
    , mLayerDataFormat(layerDataFormat)
{
}

void ChangeMapProperty::redo()
{
    swap();
}

void ChangeMapProperty::undo()
{
    swap();
}

void ChangeMapProperty::swap()
{
    Map *map = mMapDocument->map();

    switch (mProperty) {
    case Map::TileWidthProperty: {
        const int tileWidth = map->tileWidth();
        map->setTileWidth(mIntValue);
        mIntValue = tileWidth;
        break;
    }
    case Map::TileHeightProperty: {
        const int tileHeight = map->tileHeight();
        map->setTileHeight(mIntValue);
        mIntValue = tileHeight;
        break;
    }
    case Map::InfiniteProperty: {
        const int infinite = map->infinite();
        map->setInfinite(mIntValue);
        mIntValue = infinite;
        break;
    }
    case Map::OrientationProperty: {
        const Map::Orientation orientation = map->orientation();
        map->setOrientation(mOrientation);
        mOrientation = orientation;
        break;
    }
    case Map::HexSideLengthProperty: {
        const int hexSideLength = map->hexSideLength();
        map->setHexSideLength(mIntValue);
        mIntValue = hexSideLength;
        break;
    }
    case Map::StaggerAxisProperty: {
        const Map::StaggerAxis staggerAxis = map->staggerAxis();
        map->setStaggerAxis(mStaggerAxis);
        mStaggerAxis = staggerAxis;
        break;
    }
    case Map::StaggerIndexProperty: {
        const Map::StaggerIndex staggerIndex = map->staggerIndex();
        map->setStaggerIndex(mStaggerIndex);
        mStaggerIndex = staggerIndex;
        break;
    }
    case Map::ParallaxOriginProperty: {
        const QPointF parallaxOrigin = map->parallaxOrigin();
        map->setParallaxOrigin(mParallaxOrigin);
        mParallaxOrigin = parallaxOrigin;
        break;
    }
    case Map::RenderOrderProperty: {
        const Map::RenderOrder renderOrder = map->renderOrder();
        map->setRenderOrder(mRenderOrder);
        mRenderOrder = renderOrder;
        break;
    }
    case Map::BackgroundColorProperty: {
        const QColor backgroundColor = map->backgroundColor();
        map->setBackgroundColor(mBackgroundColor);
        mBackgroundColor = backgroundColor;
        break;
    }
    case Map::LayerDataFormatProperty: {
        const Map::LayerDataFormat layerDataFormat = map->layerDataFormat();
        map->setLayerDataFormat(mLayerDataFormat);
        mLayerDataFormat = layerDataFormat;
        break;
    }
    case Map::CompressionLevelProperty: {
        const int compressionLevel = map->compressionLevel();
        map->setCompressionLevel(mIntValue);
        mIntValue = compressionLevel;
        break;
    }
    case Map::ChunkSizeProperty: {
        const QSize chunkSize = map->chunkSize();
        map->setChunkSize(mChunkSize);
        mChunkSize = chunkSize;
        break;
    }
    }

    emit mMapDocument->changed(MapChangeEvent(mProperty));
    emit mMapDocument->mapChanged();
}
