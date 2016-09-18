/*
 * rtbmapobject.cpp
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

#include "rtbmapobject.h"

using namespace Tiled;


RTBMapObject::RTBMapObject()
    : mHasError(false)
    , mHasWarning(false)
    , mOriginID(0)
{

}

//=============================================================================

RTBButtonObject::RTBButtonObject():
                        mBeatsActive(mDefaults.defaultValue(BeatsActive).toInt()),
                        mLaserBeamTargets(mDefaults.defaultValue(LaserBeamTargets).toString())
{
    setObjectType(RTBMapObject::Button);

    setPropertyErrorState(BeatsActive, Nothing);
    setPropertyErrorState(LaserBeamTargets, Nothing);
}

RTBMapObject *RTBButtonObject::clone() const
{
    // no clone of the targets
    RTBButtonObject *o = new RTBButtonObject();
    o->setBeatsActive(mBeatsActive);
    o->setLaserBeamTargets(laserBeamTargets());

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBCustomFloorTrap::RTBCustomFloorTrap():
                        mIntervalSpeed(mDefaults.defaultValue(IntervalSpeed).toInt()),
                        mIntervalOffset(mDefaults.defaultValue(IntervalOffset).toInt())
{
    setObjectType(RTBMapObject::CustomFloorTrap);

    setPropertyErrorState(IntervalSpeed, Nothing);
    setPropertyErrorState(IntervalOffset, Nothing);
}

RTBMapObject *RTBCustomFloorTrap::clone() const
{
    RTBCustomFloorTrap *o = new RTBCustomFloorTrap();
    o->setIntervalSpeed(mIntervalSpeed);
    o->setIntervalOffset(mIntervalOffset);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBMovingFloorTrapSpawner::RTBMovingFloorTrapSpawner():
                        mSpawnAmount(mDefaults.defaultValue(SpawnAmount).toInt()),
                        mIntervalSpeed(mDefaults.defaultValue(IntervalSpeed).toInt()),
                        mRandomizeStart(mDefaults.defaultValue(RandomizeStart).toBool())
{
    setObjectType(RTBMapObject::MovingFloorTrapSpawner);

    setPropertyErrorState(SpawnAmount, Nothing);
    setPropertyErrorState(IntervalSpeed, Nothing);
    setPropertyErrorState(RandomizeStart, Nothing);
}

RTBMapObject *RTBMovingFloorTrapSpawner::clone() const
{
    RTBMovingFloorTrapSpawner *o = new RTBMovingFloorTrapSpawner();
    o->setSpawnAmount(mSpawnAmount);
    o->setIntervalSpeed(mIntervalSpeed);
    o->setRandomizeStart(mRandomizeStart);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBProjectileTurret::RTBProjectileTurret():
                        mIntervalSpeed(mDefaults.defaultValue(IntervalSpeed).toInt()),
                        mIntervalOffset(mDefaults.defaultValue(IntervalOffset).toInt()),
                        mProjectileSpeed(mDefaults.defaultValue(ProjectileSpeed).toInt()),
                        mShotDirection(mDefaults.defaultValue(ShotDirection).toInt())
{
    setObjectType(RTBMapObject::ProjectileTurret);

    setPropertyErrorState(IntervalSpeed, Nothing);
    setPropertyErrorState(IntervalOffset, Nothing);
    setPropertyErrorState(ProjectileSpeed, Nothing);
    setPropertyErrorState(ShotDirection, Nothing);
}

RTBMapObject *RTBProjectileTurret::clone() const
{
    RTBProjectileTurret *o = new RTBProjectileTurret();
    o->setIntervalSpeed(mIntervalSpeed);
    o->setIntervalOffset(mIntervalOffset);
    o->setProjectileSpeed(mProjectileSpeed);
    o->setShotDirection(mShotDirection);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBTeleporter::RTBTeleporter():
                        mTeleporterTarget(mDefaults.defaultValue(TeleporterTarget).toString())
{
    setObjectType(RTBMapObject::Teleporter);

    setPropertyErrorState(TeleporterTarget, Nothing);
}

RTBMapObject *RTBTeleporter::clone() const
{
    RTBTeleporter *o = new RTBTeleporter();
    o->setTeleporterTarget(mTeleporterTarget);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBLaserBeam::RTBLaserBeam():
                        mBeamType(mDefaults.defaultValue(BeamType).toInt()),
                        mActivatedOnStart(mDefaults.defaultValue(ActivatedOnStart).toBool()),
                        mDirectionDegrees(mDefaults.defaultValue(DirectionDegrees).toInt()),
                        mTargetDirectionDegrees(mDefaults.defaultValue(TargetDirectionDegrees).toInt()),
                        mIntervalOffset(mDefaults.defaultValue(IntervalOffset).toInt()),
                        mIntervalSpeed(mDefaults.defaultValue(IntervalSpeed).toInt())
{
    setObjectType(RTBMapObject::LaserBeam);

    setPropertyErrorState(BeamType, Nothing);
    setPropertyErrorState(ActivatedOnStart, Nothing);
    setPropertyErrorState(DirectionDegrees, Nothing);
    setPropertyErrorState(TargetDirectionDegrees, Nothing);
    setPropertyErrorState(IntervalOffset, Nothing);
    setPropertyErrorState(IntervalSpeed, Nothing);
}

RTBMapObject *RTBLaserBeam::clone() const
{
    RTBLaserBeam *o = new RTBLaserBeam();
    o->setBeamType(mBeamType);
    o->setActivatedOnStart(mActivatedOnStart);
    o->setDirectionDegrees(mDirectionDegrees);
    o->setTargetDirectionDegrees(mTargetDirectionDegrees);
    o->setIntervalOffset(mIntervalOffset);
    o->setIntervalSpeed(mIntervalSpeed);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBTarget::RTBTarget()
{
    setObjectType(RTBMapObject::Target);
}

RTBMapObject *RTBTarget::clone() const
{
    RTBTarget *o = new RTBTarget();
    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBFloorText::RTBFloorText()
    : mText(mDefaults.defaultValue(Text).toString())
    , mMaxCharacters(mDefaults.defaultValue(MaxCharacter).toInt())
    , mTriggerZoneSize(mDefaults.defaultValue(TriggerZone).toSizeF())
    , mUseTrigger(mDefaults.defaultValue(UseTrigger).toBool())
    , mScale(mDefaults.defaultValue(Scale).toDouble())
    , mOffsetX(mDefaults.defaultValue(OffsetX).toInt())
    , mOffsetY(mDefaults.defaultValue(OffsetY).toInt())
{
    setObjectType(RTBMapObject::FloorText);

    setPropertyErrorState(Text, Nothing);
    setPropertyErrorState(MaxCharacter, Nothing);
    setPropertyErrorState(TriggerZone, Nothing);
    setPropertyErrorState(UseTrigger, Nothing);
    setPropertyErrorState(Scale, Nothing);
    setPropertyErrorState(OffsetX, Nothing);
    setPropertyErrorState(OffsetY, Nothing);
}

RTBMapObject *RTBFloorText::clone() const
{
    RTBFloorText *o = new RTBFloorText();
    o->setText(mText);
    o->setMaxCharacters(mMaxCharacters);
    o->setTriggerZoneSize(mTriggerZoneSize);
    o->setUseTrigger(mUseTrigger);
    o->setScale(mScale);
    o->setOffsetX(mOffsetX);
    o->setOffsetY(mOffsetY);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBCameraTrigger::RTBCameraTrigger()
    : mTarget(mDefaults.defaultValue(CameraTarget).toString())
    , mTriggerZoneSize(mDefaults.defaultValue(TriggerZone).toSizeF())
    , mCameraHeight(mDefaults.defaultValue(CameraHeight).toInt())
    , mCameraAngle(mDefaults.defaultValue(CameraAngle).toInt())
{
    setObjectType(RTBMapObject::CameraTrigger);

    setPropertyErrorState(CameraTarget, Nothing);
    setPropertyErrorState(TriggerZone, Nothing);
    setPropertyErrorState(CameraHeight, Nothing);
    setPropertyErrorState(CameraAngle, Nothing);
}

RTBMapObject *RTBCameraTrigger::clone() const
{
    RTBCameraTrigger *o = new RTBCameraTrigger();
    o->setTarget(mTarget);
    o->setTriggerZoneSize(mTriggerZoneSize);
    o->setCameraHeight(mCameraHeight);
    o->setCameraAngle(mCameraAngle);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBStartLocation::RTBStartLocation()
{
    setObjectType(RTBMapObject::StartLocation);
}

RTBMapObject *RTBStartLocation::clone() const
{
    RTBStartLocation *o = new RTBStartLocation();
    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBFinishHole::RTBFinishHole()
{
    setObjectType(RTBMapObject::FinishHole);
}

RTBMapObject *RTBFinishHole::clone() const
{
    RTBFinishHole *o = new RTBFinishHole();
    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}

//=============================================================================

RTBOrb::RTBOrb()
{
    setObjectType(RTBMapObject::Orb);
}

RTBMapObject *RTBOrb::clone() const
{
    return new RTBOrb();
}

//=============================================================================

RTBNPCBallSpawner::RTBNPCBallSpawner()
    : mSpawnClass(mDefaults.defaultValue(SpawnClass).toInt())
    , mSize(mDefaults.defaultValue(Size).toInt())
    , mIntervalOffset(mDefaults.defaultValue(IntervalOffset).toInt())
    , mSpawnFrequency(mDefaults.defaultValue(SpawnFrequency).toInt())
    , mSpeed(mDefaults.defaultValue(Speed).toInt())
    , mDirection(mDefaults.defaultValue(ShotDirection).toInt())
{
    setObjectType(RTBMapObject::NPCBallSpawner);

    setPropertyErrorState(SpawnClass, Nothing);
    setPropertyErrorState(Size, Nothing);
    setPropertyErrorState(IntervalOffset, Nothing);
    setPropertyErrorState(SpawnFrequency, Nothing);
    setPropertyErrorState(Speed, Nothing);
    setPropertyErrorState(ShotDirection, Nothing);
}

RTBMapObject *RTBNPCBallSpawner::clone() const
{
    RTBNPCBallSpawner *o = new RTBNPCBallSpawner();
    o->setSpawnClass(mSpawnClass);
    o->setSize(mSize);
    o->setIntervalOffset(mIntervalOffset);
    o->setSpawnFrequency(mSpawnFrequency);
    o->setSpeed(mSpeed);
    o->setDirection(mDirection);

    if(originID() != 0)
        o->setOriginID(originID());

    return o;
}
