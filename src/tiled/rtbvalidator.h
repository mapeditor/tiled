/*
 * rtbvalidator.h
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

#ifndef RTBVALIDATOR_H
#define RTBVALIDATOR_H

#include "mapdocument.h"


namespace Tiled {

namespace Internal {

class RTBValidatorDock;
class RTBValidatorRule;

class RTBValidator : public QObject
{
    Q_OBJECT

public:
    RTBValidator(RTBValidatorDock *validatorDock);

    void setMapDocument(MapDocument *mapDocument);

    bool hasError() { return mHasError; }

    bool validate();
    void createRules();

signals:
    void highlightToolbarAction(int);

private:
    // Error
    bool checkStartLocation(RTBValidatorRule *rule);
    bool checkFinishHole(RTBValidatorRule *rule);
    bool checkTeleporterMissingTarget(RTBValidatorRule *rule);
    bool checkLaserBeamRotation(RTBValidatorRule *rule);
    bool checkObjectOverlap(RTBValidatorRule *rule);
    bool checkCameraTriggerMissingTarget(RTBValidatorRule *rule);
    bool checkLaserBeamOnWall(RTBValidatorRule *rule);
    bool checkLaserBeamBlocked(RTBValidatorRule *rule);
    bool checkObjectOnFloor(RTBValidatorRule *rule);
    bool checkObjectOverlaid(RTBValidatorRule *rule);
    bool checkFinishHoleSurrounded(RTBValidatorRule *rule);
    bool checkObjectInWall(RTBValidatorRule *rule);
    bool checkWallsAllowed(RTBValidatorRule *rule);
    bool checkObjectOnGround(RTBValidatorRule *rule);
    bool checkNPCBallSpawnerWithWalls(RTBValidatorRule *rule);
    bool checkDifficultySet(RTBValidatorRule *rule);
    bool checkPlayStyleSet(RTBValidatorRule *rule);
    bool checkPreviewImageSize(RTBValidatorRule *rule);
    bool checkProjectileTurretBlocked(RTBValidatorRule *rule);

    // Warning
    bool checkButtonTarget(RTBValidatorRule *rule);
    bool checkLaserBeamStartEndDegree(RTBValidatorRule *rule);
    bool checkDefaultValues(RTBValidatorRule *rule);
    bool checkTextMissing(RTBValidatorRule *rule);

    void findObjects();
    bool overlapAllowed(int objType);
    bool objectOnFloor(MapObject *mapObject);
    bool objectInWall(MapObject *mapObject, bool isTargetObject);
    bool objectOnGround(MapObject *mapObject);
    bool isBlocked(MapObject *mapObject, int direction);
    int getDirection(MapObject *mapObject);

    MapDocument *mMapDocument;
    RTBValidatorDock *mValidatorDock;
    QList<RTBValidatorRule*> mRules;
    RTBValidatorModel *mValidatorModel;
    bool mHasError;

    QHash<MapObject*, RTBTeleporter*> mTeleporters;
    QHash<MapObject*, RTBButtonObject*> mButtons;
    QHash<MapObject*, RTBLaserBeam*> mLaserBeams;
    QHash<MapObject*, RTBCameraTrigger*> mCameraTriggers;
    QHash<MapObject*, RTBProjectileTurret*> mProjectileTurrets;
    QHash<MapObject*, RTBStartLocation*> mStartLocations;
    QHash<MapObject*, RTBFinishHole*> mFinishHoles;
    QHash<MapObject*, RTBCustomFloorTrap*> mCustomFloorTraps;
    QHash<MapObject*, RTBMovingFloorTrapSpawner*> mMovingFloorTrapSpawners;
    QHash<MapObject*, RTBTarget*> mTargets;
    QHash<MapObject*, RTBFloorText*> mFloorTexts;
    QHash<MapObject*, RTBNPCBallSpawner*> mNPCBallSpawners;
};

} // namespace Internal
} // namespace Tiled

#endif // RTBVALIDATOR_H
