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

#include "map.h"
#include "mapdocument.h"
#include "objectgroup.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     ChangeMapProperty::Property property,
                                     int value)
    : mMapDocument(mapDocument)
    , mProperty(property)
    , mIntValue(value)
{
    switch (property) {
    case TileWidth:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Tile Width"));
        break;
    case TileHeight:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Tile Height"));
        break;
    case HexSideLength:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Hex Side Length"));
        break;
    case RTBChapter:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Base Intervall"));
        break;
    case RTBHasWalls:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Has Walls"));
        break;
    case RTBBackgroundColorScheme:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Background Color Scheme"));
        break;
    case RTBGlowColorScheme:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Glow Color Scheme"));
        break;
    case RTBLevelModifier:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Level Modifier"));
        break;
    case RTBHasStarfield:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Has Starfield"));
        break;
    case RTBDifficulty:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Difficulty"));
        break;
    case RTBPlayStyle:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Play Style"));
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
    , mProperty(BackgroundColor)
    , mBackgroundColor(backgroundColor)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::StaggerAxis staggerAxis)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Stagger Axis"))
    , mMapDocument(mapDocument)
    , mProperty(StaggerAxis)
    , mStaggerAxis(staggerAxis)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::StaggerIndex staggerIndex)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Stagger Index"))
    , mMapDocument(mapDocument)
    , mProperty(StaggerIndex)
    , mStaggerIndex(staggerIndex)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::Orientation orientation)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Orientation"))
    , mMapDocument(mapDocument)
    , mProperty(Orientation)
    , mOrientation(orientation)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::RenderOrder renderOrder)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Render Order"))
    , mMapDocument(mapDocument)
    , mProperty(RenderOrder)
    , mRenderOrder(renderOrder)
{
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     Map::LayerDataFormat layerDataFormat)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Layer Data Format"))
    , mMapDocument(mapDocument)
    , mProperty(LayerDataFormat)
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
    RTBMap *rtbMap = map->rtbMap();

    switch (mProperty) {
    case TileWidth: {
        const int tileWidth = map->tileWidth();
        map->setTileWidth(mIntValue);
        mIntValue = tileWidth;
        break;
    }
    case TileHeight: {
        const int tileHeight = map->tileHeight();
        map->setTileHeight(mIntValue);
        mIntValue = tileHeight;
        break;
    }
    case Orientation: {
        const Map::Orientation orientation = map->orientation();
        map->setOrientation(mOrientation);
        mOrientation = orientation;
        mMapDocument->createRenderer();
        break;
    }
    case HexSideLength: {
        const int hexSideLength = map->hexSideLength();
        map->setHexSideLength(mIntValue);
        mIntValue = hexSideLength;
        break;
    }
    case StaggerAxis: {
        const Map::StaggerAxis staggerAxis = map->staggerAxis();
        map->setStaggerAxis(mStaggerAxis);
        mStaggerAxis = staggerAxis;
        break;
    }
    case StaggerIndex: {
        const Map::StaggerIndex staggerIndex = map->staggerIndex();
        map->setStaggerIndex(mStaggerIndex);
        mStaggerIndex = staggerIndex;
        break;
    }
    case RenderOrder: {
        const Map::RenderOrder renderOrder = map->renderOrder();
        map->setRenderOrder(mRenderOrder);
        mRenderOrder = renderOrder;
        break;
    }
    case BackgroundColor: {
        const QColor backgroundColor = map->backgroundColor();
        map->setBackgroundColor(mBackgroundColor);
        mBackgroundColor = backgroundColor;
        break;
    }
    case LayerDataFormat: {
        const Map::LayerDataFormat layerDataFormat = map->layerDataFormat();
        map->setLayerDataFormat(mLayerDataFormat);
        mLayerDataFormat = layerDataFormat;
        break;
    }
    case RTBChapter: {
        const int baseInterval = rtbMap->chapter();
        rtbMap->setChapter(mIntValue);
        mIntValue = baseInterval;
        break;
    }
    case RTBCustomGlowColor: {
        const QColor customGlowColor = rtbMap->customGlowColor();
        rtbMap->setCustomGlowColor(mColorValue);
        mColorValue = customGlowColor;
        break;
    }
    case RTBCustomBackgroundColor: {
        const QColor customBackgroundColor = rtbMap->customBackgroundColor();
        rtbMap->setCustomBackgroundColor(mColorValue);
        mColorValue = customBackgroundColor;
        break;
    }
    case RTBLevelBrightness: {
        const double levelBrightness = rtbMap->levelBrightness();
        rtbMap->setLevelBrightness(mDoubleValue);
        mDoubleValue = levelBrightness;
        break;
    }
    case RTBCloudDensity: {
        const double cloudDensity = rtbMap->cloudDensity();
        rtbMap->setCloudDensity(mDoubleValue);
        mDoubleValue = cloudDensity;
        break;
    }
    case RTBCloudVelocity: {
        const double cloudVelocity = rtbMap->cloudVelocity();
        rtbMap->setCloudVelocity(mDoubleValue);
        mDoubleValue = cloudVelocity;
        break;
    }
    case RTBCloudAlpha: {
        const double cloudAlpha = rtbMap->cloudAlpha();
        rtbMap->setCloudAlpha(mDoubleValue);
        mDoubleValue = cloudAlpha;
        break;
    }
    case RTBSnowDensity: {
        const double snowDensity = rtbMap->snowDensity();
        rtbMap->setSnowDensity(mDoubleValue);
        mDoubleValue = snowDensity;
        break;
    }
    case RTBSnowVelocity: {
        const double snowVelocity = rtbMap->snowVelocity();
        rtbMap->setSnowVelocity(mDoubleValue);
        mDoubleValue = snowVelocity;
        break;
    }
    case RTBSnowRisingVelocity: {
        const double snowRisingVelocity = rtbMap->snowRisingVelocity();
        rtbMap->setSnowRisingVelocity(mDoubleValue);
        mDoubleValue = snowRisingVelocity;
        break;
    }
    case RTBCameraGrain: {
        const double cameraGrain = rtbMap->cameraGrain();
        rtbMap->setCameraGrain(mDoubleValue);
        mDoubleValue = cameraGrain;
        break;
    }
    case RTBCameraContrast: {
        const double cameraContrast = rtbMap->cameraContrast();
        rtbMap->setCameraContrast(mDoubleValue);
        mDoubleValue = cameraContrast;
        break;
    }
    case RTBCameraSaturation: {
        const double cameraSaturation = rtbMap->cameraSaturation();
        rtbMap->setCameraSaturation(mDoubleValue);
        mDoubleValue = cameraSaturation;
        break;
    }
    case RTBCameraGlow: {
        const double cameraGlow = rtbMap->cameraGlow();
        rtbMap->setCameraGlow(mDoubleValue);
        mDoubleValue = cameraGlow;
        break;
    }
    case RTBHasWalls: {
        const bool hasWalls = rtbMap->hasWall();
        rtbMap->setHasWall(mIntValue);
        mIntValue = hasWalls;
        // to update enable state of wall block action
        emit mMapDocument->emitHasWallsChanged();
        break;
    }
    case RTBLevelName: {
        const QString levelName = rtbMap->levelName();
        rtbMap->setLevelName(mStringValue);
        mStringValue = levelName;
        break;
    }
    case RTBLevelDescription: {
        const QString levelDescription = rtbMap->levelDescription();
        rtbMap->setLevelDescription(mStringValue);
        mStringValue = levelDescription;
        break;
    }
    case RTBBackgroundColorScheme: {
        const int backgroundColorScheme = rtbMap->backgroundColorScheme();
        rtbMap->setBackgroundColorScheme(mIntValue);
        mIntValue = backgroundColorScheme;
        break;
    }
    case RTBGlowColorScheme: {
        const int glowColorScheme = rtbMap->glowColorScheme();
        rtbMap->setGlowColorScheme(mIntValue);
        mIntValue = glowColorScheme;
        break;
    }
    case RTBLevelModifier: {
        const int levelModifier = rtbMap->levelModifier();
        rtbMap->setLevelModifier(mIntValue);
        mIntValue = levelModifier;
        break;
    }
    case RTBHasStarfield: {
        const bool hasStarfield = rtbMap->hasStarfield();
        rtbMap->setHasStarfield(mIntValue);
        mIntValue = hasStarfield;
        break;
    }
    case RTBDifficulty: {
        const int difficulty = rtbMap->difficulty();
        rtbMap->setDifficulty(mIntValue);
        mIntValue = difficulty;
        break;
    }
    case RTBPlayStyle: {
        const int playStyle = rtbMap->playStyle();
        rtbMap->setPlayStyle(mIntValue);
        mIntValue = playStyle;
        break;
    }
    case RTBPreviewImagePath: {
        const QString previewImagePath = rtbMap->previewImagePath();
        rtbMap->setPreviewImagePath(mStringValue);
        mStringValue = previewImagePath;
        break;
    }
    }

    mMapDocument->emitMapChanged();
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     ChangeMapProperty::Property property,
                                     double value)
    : mMapDocument(mapDocument)
    , mProperty(property)
    , mDoubleValue(value)
{
    switch (property) {
    case RTBCustomBaseInterval:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Custom Base Interval"));
        break;
    case RTBLevelBrightness:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Level Brightness"));
        break;
    case RTBCloudDensity:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Cloud Density"));
        break;
    case RTBCloudVelocity:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Cloud Speed"));
        break;
    case RTBCloudAlpha:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Cloud Alpha"));
        break;
    case RTBSnowDensity:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Snow Density"));
        break;
    case RTBSnowVelocity:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Snow Speed"));
        break;
    case RTBSnowRisingVelocity:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Snow Rising Velocity"));
        break;
    case RTBCameraGrain:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Camera Grain"));
        break;
    case RTBCameraContrast:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Camera Contrast"));
        break;
    case RTBCameraSaturation:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Camera Saturation"));
        break;
    case RTBCameraGlow:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Camera Glow"));
        break;
    default:
        break;
    }
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     ChangeMapProperty::Property property,
                                     QString value)
    : mMapDocument(mapDocument)
    , mProperty(property)
    , mStringValue(value)
{
    switch (property) {
    case RTBCustomMusicTrack:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Custom Music Track"));
        break;
    case RTBLevelName:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Map Name"));
        break;
    case RTBLevelDescription:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Map Description"));
        break;
    case RTBPreviewImagePath:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Preview Image Path"));
        break;
    default:
        break;
    }
}

ChangeMapProperty::ChangeMapProperty(MapDocument *mapDocument,
                                     ChangeMapProperty::Property property,
                                     const QColor &value)
    : mMapDocument(mapDocument)
    , mProperty(property)
    , mColorValue(value)
{
    switch (property) {
    case RTBCustomGlowColor:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Custom Glow Color"));
        break;
    case RTBCustomBackgroundColor:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Custom Background Color"));
        break;
    default:
        break;
    }
}
