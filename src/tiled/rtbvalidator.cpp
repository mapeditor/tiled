/*
 * rtbvalidator.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbvalidator.h"

#include "map.h"
#include "objectgroup.h"

#include "rtbmapsettings.h"
#include "rtbvalidatordock.h"
#include "rtbvalidatorrule.h"
#include "rtbvalidatormodel.h"

#include <QFile>

using namespace Tiled;
using namespace Tiled::Internal;

RTBValidator::RTBValidator(RTBValidatorDock *validatorDock)
    : mMapDocument(0)
    , mValidatorDock(validatorDock)
    , mValidatorModel(0)
    , mHasError(false)
{
    createRules();
}

void RTBValidator::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
    if(mapDocument)
        mValidatorModel = mapDocument->validatorModel();
    else
        mValidatorModel = 0;
}

void RTBValidator::createRules()
{
    // Error
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::StartLocation
                                       , QLatin1String("There must be exactly 1 Start Location in the level.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::FinishHole
                                       , QLatin1String("There must be at least 1 Finish Hole in the level.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::TeleporterMissingTarget
                                       , QLatin1String("Teleporters must have a Target.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::LaserBeamRotation
                                       , QLatin1String("Rotating Laser Beams are not allowed in levels without walls.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::CameraTriggerMissingTarget
                                       , QLatin1String("Camera Triggers must have a Target.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::LaserBeamOnWall
                                       , QLatin1String("Laser Beams must be located next to a wall (opposite to the beam direction).")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::LaserBeamBlocked
                                       , QLatin1String("Laser Beams must be blocked (e.g. by a Wall Tile or a Projectile Turret).")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::ObjectOnFloor
                                       , QLatin1String("The Object must be placed on a (grey) floor tile.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::ObjectOverlaid
                                       , QLatin1String("Objects/Orbs are not allowed to be on top of each other.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::FinishHoleSurrounded
                                       , QLatin1String("The Finish Hole must be surrounded by visible floor tiles.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::ObjectInWall
                                       , QLatin1String("The object is not allowed to be in a wall.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::WallsAllowed
                                       , QLatin1String("Wall Blocks are not allowed if the map property \"Has Walls\" is set.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::ObjectOnGround
                                       , QLatin1String("The Object must be placed on a floor tile.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::NPCBallSpawnerWithWalls
                                       , QLatin1String("Rolling Warballs are not allowed to be small in a level that has walls.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::DifficultySet
                                       , QLatin1String("Difficulty must be set.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::PlayStyleSet
                                       , QLatin1String("Play Style must be set.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::PreviewImageSize
                                       , QLatin1String("Preview Image file size must be less than 1MB.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Error, RTBValidatorRule::ProjectileTurretBlocked
                                       , QLatin1String("Projectile Turrets must be blocked (e.g. by a Wall Tile or a Projectile Turret).")));

    // Warning
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Warning, RTBValidatorRule::ButtonTarget
                                       , QLatin1String("Buttons should have a Target or they will not work.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Warning, RTBValidatorRule::LaserBeamStartEndDegree
                                       , QLatin1String("Rotating Laser Beams should have different start and end degrees.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Warning, RTBValidatorRule::ObjectOverlap
                                       , QLatin1String("Objects/Orbs should not overlap each other.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Warning, RTBValidatorRule::DefaultValues
                                       , QLatin1String("Default Value still set.")));
    mRules.append(new RTBValidatorRule(RTBValidatorRule::Warning, RTBValidatorRule::TextMissing
                                       , QLatin1String("Floor Text Object missing text.")));
}

bool RTBValidator::validate()
{
    if(!mMapDocument || !mValidatorModel)
    {
        return false;
    }

    mMapDocument->map()->rtbMap()->clearPropertyErrorState();
    findObjects();

    mHasError = false;

    for(RTBValidatorRule *rule : mRules)
    {
        switch (rule->ruleID()) {
        case RTBValidatorRule::StartLocation:
            if(!checkStartLocation(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::FinishHole:
            if(!checkFinishHole(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::TeleporterMissingTarget:
            if(!checkTeleporterMissingTarget(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::LaserBeamRotation:
            if(!checkLaserBeamRotation(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::CameraTriggerMissingTarget:
            if(!checkCameraTriggerMissingTarget(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::LaserBeamOnWall:
            if(!checkLaserBeamOnWall(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::LaserBeamBlocked:
            if(!checkLaserBeamBlocked(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::ObjectOnFloor:
            if(!checkObjectOnFloor(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::ObjectOverlaid:
            if(!checkObjectOverlaid(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::FinishHoleSurrounded:
            if(!checkFinishHoleSurrounded(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::ObjectInWall:
            if(!checkObjectInWall(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::WallsAllowed:
            if(!checkWallsAllowed(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::ObjectOnGround:
            if(!checkObjectOnGround(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::NPCBallSpawnerWithWalls:
            if(!checkNPCBallSpawnerWithWalls(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::DifficultySet:
            if(!checkDifficultySet(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::PlayStyleSet:
            if(!checkPlayStyleSet(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::PreviewImageSize:
            if(!checkPreviewImageSize(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::ProjectileTurretBlocked:
            if(!checkProjectileTurretBlocked(rule) && !mHasError)
            {
                mHasError = true;
            }
            break;
        case RTBValidatorRule::ButtonTarget:
            checkButtonTarget(rule);
            break;
        case RTBValidatorRule::LaserBeamStartEndDegree:
            checkLaserBeamStartEndDegree(rule);
            break;
        case RTBValidatorRule::ObjectOverlap:
            checkObjectOverlap(rule);
            break;
        case RTBValidatorRule::DefaultValues:
            checkDefaultValues(rule);
            break;
        case RTBValidatorRule::TextMissing:
            checkTextMissing(rule);
            break;
        default:
            break;
        }

    }

    // paint new
    mMapDocument->emitMapChanged();
    QList<MapObject*> mapObjects;
    for(Object *o : mMapDocument->currentObjects())
    {
        if(MapObject *mapObject = dynamic_cast<MapObject*>(o))
            mapObjects.append(mapObject);
    }
    mMapDocument->objectsChanged(mapObjects);
    mValidatorDock->repaint();

    return mHasError;
}

//========================== Error ==================================================================

bool RTBValidator::checkStartLocation(RTBValidatorRule *rule)
{
    if(mStartLocations.size() == 1)
        return true;
    else
    {
        mValidatorModel->appendRule(rule);
        emit highlightToolbarAction(RTBMapObject::StartLocation);
        return false;
    }
}

bool RTBValidator::checkFinishHole(RTBValidatorRule *rule)
{
    if(mFinishHoles.size() >= 1)
        return true;
    else
    {
        mValidatorModel->appendRule(rule);
        emit highlightToolbarAction(RTBMapObject::FinishHole);
        return false;
    }
}

bool RTBValidator::checkTeleporterMissingTarget(RTBValidatorRule *rule)
{
    if(mTeleporters.size() == 0)
        return true;

    bool noError = true;
    for(RTBTeleporter *teleporter : mTeleporters.values())
    {
        if(teleporter->teleporterTarget().isEmpty())
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mTeleporters.key(teleporter)));
            noError = false;
            teleporter->setPropertyErrorState(RTBMapObject::TeleporterTarget, RTBMapObject::Error);
        }
    }

    return noError;
}

bool RTBValidator::checkLaserBeamRotation(RTBValidatorRule *rule)
{
    if(mLaserBeams.size() == 0)
        return true;

    bool noError = true;
    bool hasWall = mMapDocument->map()->rtbMap()->hasWall();

    for(RTBLaserBeam *laserBeam : mLaserBeams.values())
    {
        if(laserBeam->beamType() == RTBMapObject::BT1 && !hasWall)
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mLaserBeams.key(laserBeam)));
            noError = false;
            laserBeam->setPropertyErrorState(RTBMapObject::BeamType, RTBMapObject::Error);
        }
    }

    return noError;
}

bool RTBValidator::checkCameraTriggerMissingTarget(RTBValidatorRule *rule)
{
    if(mCameraTriggers.size() == 0)
        return true;

    bool noError = true;
    for(RTBCameraTrigger *cameraTrigger : mCameraTriggers.values())
    {
        if(cameraTrigger->target().isEmpty())
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mCameraTriggers.key(cameraTrigger)));
            noError = false;
            cameraTrigger->setPropertyErrorState(RTBMapObject::CameraTarget, RTBMapObject::Error);
        }
    }

    return noError;
}

bool RTBValidator::checkLaserBeamOnWall(RTBValidatorRule *rule)
{
    if(mLaserBeams.size() == 0)
        return true;

    bool noError = true;
    bool hasWalls = mMapDocument->map()->rtbMap()->hasWall();
    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();
    QSize mapSize = mMapDocument->map()->size();

    for(RTBLaserBeam *laserBeam : mLaserBeams.values())
    {
        bool onWall = true;

        MapObject *mapObject = mLaserBeams.key(laserBeam);
        QPointF pos = mapObject->boundsUseTile().topLeft();
        int cellX = pos.x() / 32;
        int cellY = pos.y() / 32;
        Cell cell;

        switch (mapObject->cell().tile->id()) {
        case RTBMapObject::LaserBeamBottom:
        {
            if(cellY + 1 < mapSize.height())
                cell = floorLayer->cellAt(cellX, cellY + 1);
            break;
        }
        case RTBMapObject::LaserBeamLeft:
        {
            if(cellX - 1 >= 0)
                cell = floorLayer->cellAt(cellX - 1, cellY);
            break;
        }
        case RTBMapObject::LaserBeamRight:
        {
            if(cellX + 1 < mapSize.width())
                cell = floorLayer->cellAt(cellX + 1, cellY);
            break;
        }
        case RTBMapObject::LaserBeamTop:
        {
            if(cellY - 1 >= 0)
                cell = floorLayer->cellAt(cellX, cellY - 1);
            break;
        }
        }

        // if laser beam is on the border of a map
        if(!cell.tile && !hasWalls)
        {
            onWall = false;
        }
        else if(!cell.tile && hasWalls)
            continue;
        // laser beam musst be on wall or wall block, if map has no walls the laser beam musst be on a wall block
        else if((hasWalls && !cell.isEmpty() && cell.tile->id() != RTBMapSettings::WallBlock)
                || (!hasWalls && cell.isEmpty()) || (!hasWalls && cell.tile->id() != RTBMapSettings::WallBlock))
        {
            onWall = false;
        }

        // if laser beam is not on a wall create an error
        if(!onWall)
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            laserBeam->setHasError(true);

            if(noError)
                noError = false;
        }

    }

    return noError;
}

bool RTBValidator::checkLaserBeamBlocked(RTBValidatorRule *rule)
{
    // if map has walls no check needed
    if(mLaserBeams.size() == 0 || mMapDocument->map()->rtbMap()->hasWall())
        return true;

    bool noError = true;

    for(RTBLaserBeam *laserBeam : mLaserBeams.values())
    {
        if(laserBeam->beamType() == RTBMapObject::BT1)
            continue;

        MapObject *mapObject = mLaserBeams.key(laserBeam);
        if(!isBlocked(mapObject, getDirection(mapObject)))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            noError = false;
            laserBeam->setHasError(true);
        }
    }

    return noError;
}

bool RTBValidator::checkObjectOnFloor(RTBValidatorRule *rule)
{
    bool hasError = false;

    for(MapObject *mapObject : mButtons.keys())
    {
        if(!objectOnFloor(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mButtons.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mProjectileTurrets.keys())
    {
        if(!objectOnFloor(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mProjectileTurrets.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mStartLocations.keys())
    {
        if(!objectOnFloor(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mStartLocations.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mFinishHoles.keys())
    {
        if(!objectOnFloor(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mFinishHoles.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mCustomFloorTraps.keys())
    {
        if(!objectOnFloor(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mCustomFloorTraps.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mMovingFloorTrapSpawners.keys())
    {
        if(!objectOnFloor(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mMovingFloorTrapSpawners.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mFloorTexts.keys())
    {
        if(!objectOnFloor(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mFloorTexts.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    return !hasError;

}

bool RTBValidator::checkObjectOverlaid(RTBValidatorRule *rule)
{
    bool noError = true;
    QList<MapObject*> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    objects.append(mMapDocument->map()->objectGroups().at(1)->objects());

    for(MapObject *objToCheck : objects)
    {
        int objToCheckType = objToCheck->rtbMapObject()->objectType();

        // target is allowed to overlaid
        if(objToCheckType == RTBMapObject::Target)
            continue;

        QRectF bounds = objToCheck->boundsUseTile();

        for(MapObject *obj : objects)
        {
            int objType = obj->rtbMapObject()->objectType();

            // target is allowed to overlaid
            if(objType == RTBMapObject::Target)
                continue;

            if(obj != objToCheck)
            {
                // if one of the objects is no orb
                if(objToCheckType != RTBMapObject::Orb || objType != RTBMapObject::Orb)
                {
                    // Orbs are not allowed to be on the same field with ProjectileTurret and Teleporter
                    if((objToCheckType == RTBMapObject::Orb
                        && (objType != RTBMapObject::ProjectileTurret && objType != RTBMapObject::Teleporter))
                        || (objType == RTBMapObject::Orb
                        && (objToCheckType != RTBMapObject::ProjectileTurret && objToCheckType != RTBMapObject::Teleporter)))
                    continue;
                }

                if(bounds.topLeft() == obj->boundsUseTile().topLeft())
                {
                    mValidatorModel->appendRule(rule->cloneWithObject(objToCheck));
                    noError = false;
                    objToCheck->rtbMapObject()->setHasError(true);
                    break;
                }
            }
        }
    }

    return noError;
}

bool RTBValidator::checkFinishHoleSurrounded(RTBValidatorRule *rule)
{
    // if there is no finish hole to check
    if(mFinishHoles.size() == 0)
        return true;

    bool noError = true;

    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();

    for(MapObject *finishHole : mFinishHoles.keys())
    {
        bool surrounded = true;

        QPointF pos = finishHole->boundsUseTile().topLeft();
        int objCellX = pos.x() / 32;
        int objCellY = pos.y() / 32;

        int cellX = objCellX - 1;
        int cellY;

        // iterate through the 3x3 area arround the finish hole
        for(int i = 0; i <= 2; i++, cellX++)
        {
            cellY = objCellY - 1;
            for(int j = 0; j <= 2; j++, cellY++)
            {
                // if this is the cell with the finish hole no check needed
                if(cellX == objCellX && cellY == objCellY)
                    continue;

                // cell musst be on the map
                if(cellX >= 0 && cellY >= 0 && cellX < mMapDocument->map()->width()
                        && cellY < mMapDocument->map()->height())
                {
                    Cell cell = floorLayer->cellAt(cellX, cellY);
                    if(cell.isEmpty() || cell.tile->id() == RTBMapSettings::HiddenFloor)
                    {
                        surrounded = false;
                    }
                }
                else
                {
                    surrounded = false;
                }
            }

            if(!surrounded)
            {
                // if the finish hole is not surrounded
                mValidatorModel->appendRule(rule->cloneWithObject(finishHole));
                mFinishHoles.value(finishHole)->setHasError(true);
                noError = false;
                break;
            }
        }
    }

    return noError;
}

bool RTBValidator::checkObjectInWall(RTBValidatorRule *rule)
{
    if(!mMapDocument->map()->rtbMap()->hasWall())
        return true;

    bool hasError = false;

    for(MapObject *mapObject : mTeleporters.keys())
    {
        if(objectInWall(mapObject, false))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mTeleporters.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mTargets.keys())
    {
        if(objectInWall(mapObject, true))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mTargets.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    for(MapObject *mapObject : mFloorTexts.keys())
    {
        if(objectInWall(mapObject, false))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mFloorTexts.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    return !hasError;
}

bool RTBValidator::checkWallsAllowed(RTBValidatorRule *rule)
{
    if(!mMapDocument->map()->rtbMap()->hasWall())
        return true;

    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();

    for(int i = 0; i < mMapDocument->map()->width(); i++)
    {
        for(int j = 0; j < mMapDocument->map()->height(); j++)
        {
            Cell cell = floorLayer->cellAt(i, j);

            if(!cell.isEmpty() && cell.tile->id() == RTBMapSettings::WallBlock)
            {
                mValidatorModel->appendRule(rule);
                return false;
            }
        }
    }

    return true;
}

bool RTBValidator::checkObjectOnGround(RTBValidatorRule *rule)
{
    bool hasError = false;

    for(MapObject *mapObject : mNPCBallSpawners.keys())
    {
        if(!objectOnGround(mapObject))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mNPCBallSpawners.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    // no check needed if map has no wall, than lbs in the "air" allowed if they hang on a wall block
    if(mMapDocument->map()->rtbMap()->hasWall())
    {
        for(MapObject *mapObject : mLaserBeams.keys())
        {
            if(!objectOnGround(mapObject))
            {
                mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
                mLaserBeams.value(mapObject)->setHasError(true);

                if(!hasError)
                    hasError = true;
            }
        }
    }

    return !hasError;
}

bool RTBValidator::checkNPCBallSpawnerWithWalls(RTBValidatorRule *rule)
{
    if(!mMapDocument->map()->rtbMap()->hasWall())
        return true;

    bool hasError = false;

    for(MapObject *mapObject : mNPCBallSpawners.keys())
    {
        RTBNPCBallSpawner *ballSpawner = static_cast<RTBNPCBallSpawner*>(mapObject->rtbMapObject());
        if(ballSpawner->size() ==  RTBMapObject::SMALL && ballSpawner->spawnClass() == RTBMapObject::SC0)
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            mNPCBallSpawners.value(mapObject)->setHasError(true);

            if(!hasError)
                hasError = true;
        }
    }

    return !hasError;
}

bool RTBValidator::checkDifficultySet(RTBValidatorRule *rule)
{
    bool hasError = false;
    RTBMap *rtbMap = mMapDocument->map()->rtbMap();

    if(rtbMap->difficulty() == 0){
        mValidatorModel->appendRule(rule);
        rtbMap->setPropertyErrorState(RTBMap::Difficulty, RTBMap::Error);

        if(!hasError)
            hasError = true;
    }

    return !hasError;
}

bool RTBValidator::checkPlayStyleSet(RTBValidatorRule *rule)
{
    bool hasError = false;
    RTBMap *rtbMap = mMapDocument->map()->rtbMap();

    if(rtbMap->playStyle() == 0){
        mValidatorModel->appendRule(rule);
        rtbMap->setPropertyErrorState(RTBMap::PlayStyle, RTBMap::Error);

        if(!hasError)
            hasError = true;
    }

    return !hasError;
}

bool RTBValidator::checkPreviewImageSize(RTBValidatorRule *rule)
{
    RTBMap *rtbMap = mMapDocument->map()->rtbMap();

    if(rtbMap->previewImagePath().isEmpty()){
        return true;
    }

    QFile file(rtbMap->previewImagePath());
    if(file.size() > (1024 * 1024)){
        mValidatorModel->appendRule(rule);
        rtbMap->setPropertyErrorState(RTBMap::PreviewImagePath, RTBMap::Error);
        return false;
    }


    return true;
}

bool RTBValidator::checkProjectileTurretBlocked(RTBValidatorRule *rule)
{
    // if map has walls no check needed
    if(mProjectileTurrets.size() == 0 || mMapDocument->map()->rtbMap()->hasWall())
        return true;

    bool noError = true;

    for(RTBProjectileTurret *projectileTurret : mProjectileTurrets.values())
    {
        MapObject *mapObject = mProjectileTurrets.key(projectileTurret);
        int direction = getDirection(mapObject);
        if(direction == RTBMapObject::All){
            for(int i = 0; i < RTBMapObject::All; i++){
                if(!isBlocked(mapObject, i))
                {
                    mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
                    noError = false;
                    projectileTurret->setHasError(true);
                    break;
                }
            }
        }
        else if(!isBlocked(mapObject, getDirection(mapObject)))
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mapObject));
            noError = false;
            projectileTurret->setHasError(true);
        }
    }

    return noError;
}

//========================== Warning ================================================================

bool RTBValidator::checkButtonTarget(RTBValidatorRule *rule)
{
    if(mButtons.size() == 0)
        return true;

    bool noError = true;

    for(RTBButtonObject *button : mButtons.values())
    {
        if(button->laserBeamTargets().isEmpty())
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mButtons.key(button)));
            noError = false;
            button->setPropertyErrorState(RTBMapObject::LaserBeamTargets, RTBMapObject::Warning);
        }
    }

    return noError;
}

bool RTBValidator::checkLaserBeamStartEndDegree(RTBValidatorRule *rule)
{
    if(mLaserBeams.size() == 0)
        return true;

    bool noError = true;

    for(RTBLaserBeam *laserBeam : mLaserBeams.values())
    {
        if(laserBeam->beamType() == RTBMapObject::BT1
                && laserBeam->directionDegrees() == 0 && laserBeam->targetDirectionDegrees() == 0)
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mLaserBeams.key(laserBeam)));
            noError = false;
            laserBeam->setPropertyErrorState(RTBMapObject::DirectionDegrees, RTBMapObject::Warning);
            laserBeam->setPropertyErrorState(RTBMapObject::TargetDirectionDegrees, RTBMapObject::Warning);
        }
    }

    return noError;
}

bool RTBValidator::checkObjectOverlap(RTBValidatorRule *rule)
{
    bool noError = true;
    QList<MapObject*> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    objects.append(mMapDocument->map()->objectGroups().at(1)->objects());

    for(MapObject *objToCheck : objects)
    {
        QRectF bounds = objToCheck->boundsUseTile();
        int objToCheckType = objToCheck->rtbMapObject()->objectType();

        if(overlapAllowed(objToCheckType))
            continue;

        for(MapObject *obj : objects)
        {
            if(obj != objToCheck)
            {
                int objType = obj->rtbMapObject()->objectType();

                if(overlapAllowed(objType))
                    continue;

                // if one of the objects is no orb
                if(objToCheckType != RTBMapObject::Orb || objType != RTBMapObject::Orb)
                {
                    // Orbs are not allowed to be on the same field with ProjectileTurret and Teleporter
                    if((objToCheckType == RTBMapObject::Orb
                        && (objType != RTBMapObject::ProjectileTurret && objType != RTBMapObject::Teleporter))
                        || (objType == RTBMapObject::Orb
                        && (objToCheckType != RTBMapObject::ProjectileTurret && objToCheckType != RTBMapObject::Teleporter)))
                    continue;
                }

                if(bounds.intersects(obj->boundsUseTile())
                        && bounds.topLeft() != obj->boundsUseTile().topLeft())
                {
                    mValidatorModel->appendRule(rule->cloneWithObject(objToCheck));
                    noError = false;
                    objToCheck->rtbMapObject()->setHasWarning(true);
                    break;
                }
            }
        }
    }

    return noError;
}

bool RTBValidator::checkDefaultValues(RTBValidatorRule *rule)
{
    bool noError = true;
    RTBMap *rtbMap = mMapDocument->map()->rtbMap();

    // level description
    if(rtbMap->levelDescription().isEmpty())
    {
        RTBValidatorRule *levelDescriptionRule = rule->clone();
        levelDescriptionRule->setMessage(QLatin1String("Level Description: ") + rule->message());
        mValidatorModel->appendRule(levelDescriptionRule);
        rtbMap->setPropertyErrorState(RTBMap::LevelDescription, RTBMap::Warning);

        if(noError)
            noError = false;
    }

    return noError;
}

bool RTBValidator::checkTextMissing(RTBValidatorRule *rule)
{
    if(mFloorTexts.size() == 0)
        return true;

    bool noError = true;

    for(RTBFloorText *floorText : mFloorTexts.values())
    {
        if(floorText->text().isEmpty())
        {
            mValidatorModel->appendRule(rule->cloneWithObject(mFloorTexts.key(floorText)));
            noError = false;
            floorText->setPropertyErrorState(RTBMapObject::Text, RTBMapObject::Warning);
        }
    }

    return noError;
}

bool RTBValidator::overlapAllowed(int objType)
{
    if(objType == RTBMapObject::Target || objType == RTBMapObject::CameraTrigger
            || objType == RTBMapObject::FloorText || objType == RTBMapObject::StartLocation)
        return true;

    return false;
}

void RTBValidator::findObjects()
{
    mTeleporters.clear();
    mButtons.clear();
    mLaserBeams.clear();
    mCameraTriggers.clear();
    mProjectileTurrets.clear();
    mStartLocations.clear();
    mFinishHoles.clear();
    mCustomFloorTraps.clear();
    mMovingFloorTrapSpawners.clear();
    mTargets.clear();
    mFloorTexts.clear();
    mNPCBallSpawners.clear();

    mValidatorModel->clearRules();

    QList<MapObject*> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    objects.append(mMapDocument->map()->objectGroups().at(1)->objects());

    for(MapObject *obj : objects)
    {
        RTBMapObject *rtbMapObject = obj->rtbMapObject();
        rtbMapObject->clearErrorState();

        switch (rtbMapObject->objectType()) {
        case RTBMapObject::Teleporter:
        {
            RTBTeleporter *teleporter = static_cast<RTBTeleporter*>(rtbMapObject);
            mTeleporters.insert(obj, teleporter);
            break;
        }
        case RTBMapObject::Button:
        {
            RTBButtonObject *button = static_cast<RTBButtonObject*>(rtbMapObject);
            mButtons.insert(obj, button);
            break;
        }
        case RTBMapObject::LaserBeam:
        {
            RTBLaserBeam *laserBeam = static_cast<RTBLaserBeam*>(rtbMapObject);
            mLaserBeams.insert(obj, laserBeam);
            break;
        }
        case RTBMapObject::CameraTrigger:
        {
            RTBCameraTrigger *cameraTrigger = static_cast<RTBCameraTrigger*>(rtbMapObject);
            mCameraTriggers.insert(obj, cameraTrigger);
            break;
        }
        case RTBMapObject::ProjectileTurret:
        {
            RTBProjectileTurret *projectileTurret = static_cast<RTBProjectileTurret*>(rtbMapObject);
            mProjectileTurrets.insert(obj, projectileTurret);
            break;
        }
        case RTBMapObject::StartLocation:
        {
            RTBStartLocation *startLocation = static_cast<RTBStartLocation*>(rtbMapObject);
            mStartLocations.insert(obj, startLocation);
            break;
        }
        case RTBMapObject::FinishHole:
        {
            RTBFinishHole *finishHole = static_cast<RTBFinishHole*>(rtbMapObject);
            mFinishHoles.insert(obj, finishHole);
            break;
        }
        case RTBMapObject::CustomFloorTrap:
        {
            RTBCustomFloorTrap *customFloorTrap = static_cast<RTBCustomFloorTrap*>(rtbMapObject);
            mCustomFloorTraps.insert(obj, customFloorTrap);
            break;
        }
        case RTBMapObject::MovingFloorTrapSpawner:
        {
            RTBMovingFloorTrapSpawner *movingFloorTrapSpawner = static_cast<RTBMovingFloorTrapSpawner*>(rtbMapObject);
            mMovingFloorTrapSpawners.insert(obj, movingFloorTrapSpawner);
            break;
        }
        case RTBMapObject::Target:
        {
            RTBTarget *target = static_cast<RTBTarget*>(rtbMapObject);
            mTargets.insert(obj, target);
            break;
        }
        case RTBMapObject::FloorText:
        {
            RTBFloorText *floorText = static_cast<RTBFloorText*>(rtbMapObject);
            mFloorTexts.insert(obj, floorText);
            break;
        }
        case RTBMapObject::NPCBallSpawner:
        {
            RTBNPCBallSpawner *npcBallSpawner = static_cast<RTBNPCBallSpawner*>(rtbMapObject);
            mNPCBallSpawners.insert(obj, npcBallSpawner);
            break;
        }
        default:
            break;
        }
    }
}

bool RTBValidator::objectOnFloor(MapObject *mapObject)
{
    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();
    QPointF pos = mapObject->boundsUseTile().topLeft();
    qreal cellX = pos.x() / 32;
    qreal cellY = pos.y() / 32;

    // if the object is outside the map no check needed
    if(cellX < 0 || cellY < 0 || cellX > mMapDocument->map()->width()
            || cellY > mMapDocument->map()->height())
        return true;

    // if the object is on only one cell
    if(cellX == floor(cellX) && cellY == floor(cellY))
    {
        Cell cell = floorLayer->cellAt(cellX, cellY);
        if(!cell.isEmpty() && cell.tile->id() == RTBMapSettings::Floor)
            return true;
    }
    // if the object is shifted up/down
    else if(cellX == floor(cellX))
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomLeft();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomLeft = floorLayer->cellAt(cellX, cellY);

        if(!cellTopLeft.isEmpty() && cellTopLeft.tile->id() == RTBMapSettings::Floor
                && !cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() == RTBMapSettings::Floor)
            return true;
    }
    // if the object is shifted left/right
    else if(cellY == floor(cellY))
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().topRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellTopRight = floorLayer->cellAt(cellX, cellY);


        if(!cellTopLeft.isEmpty() && cellTopLeft.tile->id() == RTBMapSettings::Floor
                && !cellTopRight.isEmpty() && cellTopRight.tile->id() == RTBMapSettings::Floor)
            return true;
    }
    // if the center of the object is on the edge of a cell
    else
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().topRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellTopRight = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomLeft();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomRight = floorLayer->cellAt(cellX, cellY);

        if(!cellTopLeft.isEmpty() && cellTopLeft.tile->id() == RTBMapSettings::Floor
                && !cellTopRight.isEmpty() && cellTopRight.tile->id() == RTBMapSettings::Floor
                && !cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() == RTBMapSettings::Floor
                && !cellBottomRight.isEmpty() && cellBottomRight.tile->id() == RTBMapSettings::Floor)
            return true;
    }

    return false;
}

bool RTBValidator::objectInWall(MapObject *mapObject, bool isTargetObject)
{
    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();
    QPointF pos = mapObject->boundsUseTile().topLeft();
    qreal cellX = pos.x() / 32;
    qreal cellY = pos.y() / 32;

    // if the object is outside the map no check needed
    if(cellX < 0 || cellY < 0 || cellX > mMapDocument->map()->width()
            || cellY > mMapDocument->map()->height())
        return false;

    // if the object is on only one cell
    if(cellX == floor(cellX) && cellY == floor(cellY))
    {
            return false;
    }
    // if the object is shifted up/down
    else if(cellX == floor(cellX))
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomLeft();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomLeft = floorLayer->cellAt(cellX, cellY);

        // target objects are allowed to be on the edge if they are on an hidden tile
        if(isTargetObject)
        {
            if(!cellTopLeft.isEmpty() && cellTopLeft.tile->id() == RTBMapSettings::HiddenFloor  && cellBottomLeft.isEmpty()
                    || !cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() == RTBMapSettings::HiddenFloor && cellTopLeft.isEmpty())
                return false;
            else if(!cellTopLeft.isEmpty() && !cellBottomLeft.isEmpty())
            {
                if(cellTopLeft.tile->id() != RTBMapSettings::HiddenFloor
                    && cellBottomLeft.tile->id() == RTBMapSettings::HiddenFloor
                    || cellBottomLeft.tile->id() != RTBMapSettings::HiddenFloor
                    && cellTopLeft.tile->id() == RTBMapSettings::HiddenFloor)
                return true;
            }
        }

        if(cellTopLeft.isEmpty() && cellBottomLeft.isEmpty()
                || !cellTopLeft.isEmpty() && !cellBottomLeft.isEmpty())
            return false;
        else
            return true;
    }
    // if the object is shifted left/right
    else if(cellY == floor(cellY))
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().topRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellTopRight = floorLayer->cellAt(cellX, cellY);

        // target objects are allowed to be on the edge if they are on an hidden tile
        if(isTargetObject)
        {
            if(!cellTopLeft.isEmpty() && cellTopLeft.tile->id() == RTBMapSettings::HiddenFloor  && cellTopRight.isEmpty()
                    || !cellTopRight.isEmpty() && cellTopRight.tile->id() == RTBMapSettings::HiddenFloor && cellTopLeft.isEmpty())
                return false;
            else if(!cellTopLeft.isEmpty() && !cellTopRight.isEmpty())
            {
                if(cellTopLeft.tile->id() != RTBMapSettings::HiddenFloor
                    && cellTopRight.tile->id() == RTBMapSettings::HiddenFloor
                    || cellTopRight.tile->id() != RTBMapSettings::HiddenFloor
                    && cellTopLeft.tile->id() == RTBMapSettings::HiddenFloor)
                return true;
            }
        }

        if(cellTopLeft.isEmpty() && cellTopRight.isEmpty()
                || !cellTopLeft.isEmpty() && !cellTopRight.isEmpty())
            return false;
        else
            return true;
    }
    // if the center of the object is on the edge of a cell
    else
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().topRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellTopRight = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomLeft();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomRight = floorLayer->cellAt(cellX, cellY);

        // target objects are allowed to be on the edge if they are on an hidden tile
        if(isTargetObject)
        {
            if((!cellTopLeft.isEmpty() && cellTopLeft.tile->id() == RTBMapSettings::HiddenFloor  || cellTopLeft.isEmpty())
                    && (!cellTopRight.isEmpty() && cellTopRight.tile->id() == RTBMapSettings::HiddenFloor  || cellTopRight.isEmpty())
                    && (!cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() == RTBMapSettings::HiddenFloor  || cellBottomLeft.isEmpty())
                    && (!cellBottomRight.isEmpty() && cellBottomRight.tile->id() == RTBMapSettings::HiddenFloor  || cellBottomRight.isEmpty()))
                return false;
            else if(!cellTopLeft.isEmpty() && cellTopLeft.tile->id() == RTBMapSettings::HiddenFloor
                    && (!cellTopRight.isEmpty() && cellTopRight.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellBottomRight.isEmpty() && cellBottomRight.tile->id() != RTBMapSettings::HiddenFloor))
                return true;
            else if(!cellTopRight.isEmpty() && cellTopRight.tile->id() == RTBMapSettings::HiddenFloor
                    && (!cellTopLeft.isEmpty() && cellTopLeft.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellBottomRight.isEmpty() && cellBottomRight.tile->id() != RTBMapSettings::HiddenFloor))
                return true;
            else if(!cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() == RTBMapSettings::HiddenFloor
                    && (!cellTopLeft.isEmpty() && cellTopLeft.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellTopRight.isEmpty() && cellTopRight.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellBottomRight.isEmpty() && cellBottomRight.tile->id() != RTBMapSettings::HiddenFloor))
                return true;
            else if(!cellBottomRight.isEmpty() && cellBottomRight.tile->id() == RTBMapSettings::HiddenFloor
                    && (!cellTopLeft.isEmpty() && cellTopLeft.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellBottomLeft.isEmpty() && cellBottomLeft.tile->id() != RTBMapSettings::HiddenFloor
                    || !cellTopRight.isEmpty() && cellTopRight.tile->id() != RTBMapSettings::HiddenFloor))
                return true;
        }

        if(cellTopLeft.isEmpty() && cellTopRight.isEmpty() && cellBottomLeft.isEmpty() && cellBottomRight.isEmpty()
                || !cellTopLeft.isEmpty() && !cellTopRight.isEmpty() && !cellBottomLeft.isEmpty() && !cellBottomRight.isEmpty())
            return false;
        else
            return true;
    }

    return false;
}

bool RTBValidator::objectOnGround(MapObject *mapObject)
{
    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();
    QPointF pos = mapObject->boundsUseTile().topLeft();
    qreal cellX = pos.x() / 32;
    qreal cellY = pos.y() / 32;

    // if the object is outside the map no check needed
    if(cellX < 0 || cellY < 0 || cellX > mMapDocument->map()->width()
            || cellY > mMapDocument->map()->height())
        return true;

    // if the object is on only one cell
    if(cellX == floor(cellX) && cellY == floor(cellY))
    {
        Cell cell = floorLayer->cellAt(cellX, cellY);
        if(!cell.isEmpty())
            return true;
    }
    // if the object is shifted up/down
    else if(cellX == floor(cellX))
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomLeft();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomLeft = floorLayer->cellAt(cellX, cellY);

        if(!cellTopLeft.isEmpty() && !cellBottomLeft.isEmpty())
            return true;
    }
    // if the object is shifted left/right
    else if(cellY == floor(cellY))
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().topRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellTopRight = floorLayer->cellAt(cellX, cellY);


        if(!cellTopLeft.isEmpty() && !cellTopRight.isEmpty())
            return true;
    }
    // if the center of the object is on the edge of a cell
    else
    {
        Cell cellTopLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().topRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellTopRight = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomLeft();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomLeft = floorLayer->cellAt(cellX, cellY);

        pos = mapObject->boundsUseTile().bottomRight();
        cellX = pos.x() / 32;
        cellY = pos.y() / 32;
        Cell cellBottomRight = floorLayer->cellAt(cellX, cellY);

        if(!cellTopLeft.isEmpty() && !cellTopRight.isEmpty()
                && !cellBottomLeft.isEmpty() && !cellBottomRight.isEmpty())
            return true;
    }

    return false;
}

bool RTBValidator::isBlocked(MapObject *mapObject, int direction){
    TileLayer *floorLayer = mMapDocument->map()->layerAt(RTBMapSettings::FloorID)->asTileLayer();
    QPointF pos = mapObject->boundsUseTile().topLeft();
    int cellX = pos.x() / 32;
    int cellY = pos.y() / 32;
    Cell cell;
    int originCellX = cellX;
    int originCellY = cellY;

    switch (direction) {
    case RTBMapObject::Up:
        for(; cellY > 0 ; cellY--)
        {
            cell = floorLayer->cellAt(cellX, cellY - 1);
            if(!cell.isEmpty() && cell.tile->id() == RTBMapSettings::WallBlock)
            {
                return true;
            }
        }
        break;
    case RTBMapObject::Right:
        for(; cellX < mMapDocument->map()->width()-1; cellX++)
        {
            cell = floorLayer->cellAt(cellX + 1, cellY);
            if(!cell.isEmpty() && cell.tile->id() == RTBMapSettings::WallBlock)
            {
                return true;
            }
        }
        break;
    case RTBMapObject::Left:
        for(; cellX > 0 ; cellX--)
        {
            cell = floorLayer->cellAt(cellX - 1, cellY);
            if(!cell.isEmpty() && cell.tile->id() == RTBMapSettings::WallBlock)
            {
                return true;
            }
        }
        break;
    case RTBMapObject::Down:
        for(; cellY < mMapDocument->map()->height()-1; cellY++)
        {
            cell = floorLayer->cellAt(cellX, cellY + 1);
            if(!cell.isEmpty() && cell.tile->id() == RTBMapSettings::WallBlock)
            {
                return true;
            }
        }
        break;
    }

    // if no wall block found which block the laser search for projectile turret
    for(RTBProjectileTurret *projectileTurret : mProjectileTurrets.values())
    {
       if(mProjectileTurrets.key(projectileTurret) == mapObject){
           continue;
       }

        MapObject *projectileTurretObject = mProjectileTurrets.key(projectileTurret);
        QPointF pos = projectileTurretObject->boundsUseTile().topLeft();
        qreal projectileTurretCellX = pos.x() / 32;
        qreal projectileTurretCellY = pos.y() / 32;

        switch (direction) {
        case RTBMapObject::Down:
            if(originCellX == projectileTurretCellX && originCellY < projectileTurretCellY)
                return true;
            break;
        case RTBMapObject::Left:
            if(originCellY == projectileTurretCellY && originCellX > projectileTurretCellX)
                return true;
            break;
        case RTBMapObject::Right:
            if(originCellY == projectileTurretCellY && originCellX < projectileTurretCellX)
                return true;
            break;
        case RTBMapObject::Up:
            if(originCellX == projectileTurretCellX && originCellY > projectileTurretCellY)
                return true;
            break;
        }
    }

    return false;
}

int RTBValidator::getDirection(MapObject *mapObject){
    switch (mapObject->cell().tile->id()) {
    case RTBMapObject::LaserBeamBottom:
        return RTBMapObject::Up;
    case RTBMapObject::LaserBeamRight:
        return RTBMapObject::Left;
    case RTBMapObject::LaserBeamLeft:
        return RTBMapObject::Right;
    case RTBMapObject::LaserBeamTop:
        return RTBMapObject::Down;
    case RTBMapObject::ProjectileTurret:{
        RTBProjectileTurret *projectilteTurret = static_cast<RTBProjectileTurret*>(mapObject->rtbMapObject());
        return projectilteTurret->shotDirection();
    }
    default:
        return RTBMapObject::Up;
    }
}
