/*
 * rtbchangemapobjectproperties.cpp
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

#include "rtbchangemapobjectproperties.h"

#include "map.h"
#include "mapobjectmodel.h"
#include "objectgroup.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

RTBChangeMapObjectProperties::RTBChangeMapObjectProperties(MapDocument *mapDocument,
                                        MapObject *mapObject,
                                        RTBChangeMapObjectProperties::Property property,
                                        int value)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mProperty(property)
    , mIntValue(value)
{
    switch (property) {
    case RTBIntervalSpeed:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Interval Speed"));
        break;
    case RTBIntervalOffset:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Interval Offset"));
        break;
    case RTBSpawnAmount:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Spawn Amount"));
        break;
    case RTBBeatsActive:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Beats Active"));
        break;
    case RTBBeamType:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Beam Type"));
        break;
    case RTBActivatedOnStart:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Activated On Start"));
        break;
    case RTBDirectionDegrees:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Direction Degrees"));
        break;
    case RTBTargetDirectionDegrees:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Target Direction Degrees"));
        break;
    case RTBProjectileSpeed:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Projectile Speed"));
        break;
    case RTBShotDirection:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Shot Direction"));
        break;
    case RTBTeleporterTarget:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Teleporter Target"));
        break;
    case RTBUseTrigger:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Use Trigger"));
        break;
    case RTBCameraTarget:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Camera Target"));
        break;
    case RTBCameraHeight:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Camera Height"));
        break;
    case RTBCameraAngle:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Camera Angle"));
        break;
    case RTBMaxCharacters:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Max Characters per Line"));
        break;
    case RTBRandomizeStart:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Randomize Start"));
        break;
    case RTBSpawnClass:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Spawn Class"));
        break;
    case RTBSize:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Size"));
        break;
    case RTBSpawnFrequency:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Spawn Frequency"));
        break;
    case RTBSpeed:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Speed"));
        break;
    default:
        break;
    }
}

RTBChangeMapObjectProperties::RTBChangeMapObjectProperties(MapDocument *mapDocument,
                                        MapObject *mapObject,
                                        RTBChangeMapObjectProperties::Property property,
                                        QString value)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mProperty(property)
    , mStringValue(value)
{
    switch (property) {
    case RTBLaserBeamTargets:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Laser Beam Targets"));
        break;
    case RTBText:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Text"));
        break;
    }
}

RTBChangeMapObjectProperties::RTBChangeMapObjectProperties(MapDocument *mapDocument,
                                        MapObject *mapObject,
                                        RTBChangeMapObjectProperties::Property property,
                                        QSizeF value)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mProperty(property)
    , mSizeValue(value)
{
    switch (property) {
    case RTBTriggerZone:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Trigger Zone"));
        break;
    }

}

RTBChangeMapObjectProperties::RTBChangeMapObjectProperties(MapDocument *mapDocument,
                                        MapObject *mapObject,
                                        RTBChangeMapObjectProperties::Property property,
                                        double value)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mProperty(property)
    , mDoubleValue(value)
{
    switch (property) {
    case RTBScale:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Scale"));
        break;
    case RTBOffsetX:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Offset X"));
        break;
    case RTBOffsetY:
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Offset Y"));
        break;
    }
}

void RTBChangeMapObjectProperties::redo()
{
    swap();
}

void RTBChangeMapObjectProperties::undo()
{
    swap();
}

void RTBChangeMapObjectProperties::swap()
{
    RTBMapObject *rtbMapObject = mMapObject->rtbMapObject();
    switch (rtbMapObject->objectType()) {
    case RTBMapObject::CustomFloorTrap:
    {
        swap(static_cast<RTBCustomFloorTrap*>(rtbMapObject));
        break;
    }
    case RTBMapObject::MovingFloorTrapSpawner:
    {
        swap(static_cast<RTBMovingFloorTrapSpawner*>(rtbMapObject));
        break;
    }
    case RTBMapObject::Button:
    {
        swap(static_cast<RTBButtonObject*>(rtbMapObject));
        break;
    }
    case RTBMapObject::LaserBeam:
    {
        swap(static_cast<RTBLaserBeam*>(rtbMapObject));
        break;
    }
    case RTBMapObject::ProjectileTurret:
    {
        swap(static_cast<RTBProjectileTurret*>(rtbMapObject));
        break;
    }
    case RTBMapObject::Teleporter:
    {
        swap(static_cast<RTBTeleporter*>(rtbMapObject));
        break;
    }
    case RTBMapObject::FloorText:
    {
        swap(static_cast<RTBFloorText*>(rtbMapObject));
        break;
    }
    case RTBMapObject::CameraTrigger:
    {
        swap(static_cast<RTBCameraTrigger*>(rtbMapObject));
        break;
    }
    case RTBMapObject::NPCBallSpawner:
    {
        swap(static_cast<RTBNPCBallSpawner*>(rtbMapObject));
        break;
    }
    case RTBMapObject::Target:
    {
        return;
    }
    default:

        return;
    }

    mMapDocument->mapObjectModel()->emitObjectsChanged(mMapDocument->selectedObjects());
}

void RTBChangeMapObjectProperties::swap(RTBCustomFloorTrap *rtbMapObject)
{
    switch (mProperty) {
    case RTBIntervalSpeed: {
        const int intervalSpeed = rtbMapObject->intervalSpeed();
        rtbMapObject->setIntervalSpeed(mIntValue);
        mIntValue = intervalSpeed;
        break;
    }
    case RTBIntervalOffset: {
        const int intervalOffset = rtbMapObject->intervalOffset();
        rtbMapObject->setIntervalOffset(mIntValue);
        mIntValue = intervalOffset;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBMovingFloorTrapSpawner *rtbMapObject)
{
    switch (mProperty) {
    case RTBSpawnAmount: {
        const int spawnAmount = rtbMapObject->spawnAmount();
        rtbMapObject->setSpawnAmount(mIntValue);
        mIntValue = spawnAmount;
        break;
    }
    case RTBIntervalSpeed: {
        const int intervalSpeed = rtbMapObject->intervalSpeed();
        rtbMapObject->setIntervalSpeed(mIntValue);
        mIntValue = intervalSpeed;
        break;
    }
    case RTBRandomizeStart: {
        const int randomizeStart = rtbMapObject->randomizeStart();
        rtbMapObject->setRandomizeStart(mIntValue);
        mIntValue = randomizeStart;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBButtonObject *rtbMapObject)
{
    switch (mProperty) {
    case RTBBeatsActive: {
        const int beatsActive = rtbMapObject->beatsActive();
        rtbMapObject->setBeatsActive(mIntValue);
        mIntValue = beatsActive;
        break;
    }
    case RTBLaserBeamTargets: {
        // no changes possible
        //const QString laserBeamTargets = rtbMapObject->laserBeamTargets();
        //rtbMapObject->setLaserBeamTargets(mStringValue);
        //mStringValue = laserBeamTargets;
        break;
    }
    case RTBLaserBeamTarget1: {
        const QString laserBeamTarget1 = rtbMapObject->target(RTBLaserBeamTarget1);
        rtbMapObject->insertTarget(RTBLaserBeamTarget1, mStringValue);
        mStringValue = laserBeamTarget1;
        break;
    }
    case RTBLaserBeamTarget2: {
        const QString laserBeamTarget2 = rtbMapObject->target(RTBLaserBeamTarget2);
        rtbMapObject->insertTarget(RTBLaserBeamTarget2, mStringValue);
        mStringValue = laserBeamTarget2;
        break;
    }
    case RTBLaserBeamTarget3: {
        const QString laserBeamTarget3 = rtbMapObject->target(RTBLaserBeamTarget3);
        rtbMapObject->insertTarget(RTBLaserBeamTarget3, mStringValue);
        mStringValue = laserBeamTarget3;
        break;
    }
    case RTBLaserBeamTarget4: {
        const QString laserBeamTarget4 = rtbMapObject->target(RTBLaserBeamTarget4);
        rtbMapObject->insertTarget(RTBLaserBeamTarget4, mStringValue);
        mStringValue = laserBeamTarget4;
        break;
    }
    case RTBLaserBeamTarget5: {
        const QString laserBeamTarget5 = rtbMapObject->target(RTBLaserBeamTarget5);
        rtbMapObject->insertTarget(RTBLaserBeamTarget5, mStringValue);
        mStringValue = laserBeamTarget5;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBLaserBeam *rtbMapObject)
{
    switch (mProperty) {
    case RTBBeamType: {
        const int beamType = rtbMapObject->beamType();
        rtbMapObject->setBeamType(mIntValue);
        mIntValue = beamType;

        if(rtbMapObject->beamType() == RTBMapObject::BT2)
            removeLaserFromRelatedButton();
        else if(mIntValue == RTBMapObject::BT2)
            addLaserToRelatedButton();
        break;
    }
    case RTBActivatedOnStart: {
        const int activatedOnStart = rtbMapObject->activatedOnStart();
        rtbMapObject->setActivatedOnStart(mIntValue);
        mIntValue = activatedOnStart;
        break;
    }
    case RTBDirectionDegrees: {
        const int directionDegrees = rtbMapObject->directionDegrees();
        rtbMapObject->setDirectionDegrees(mIntValue);
        mIntValue = directionDegrees;
        break;
    }
    case RTBTargetDirectionDegrees: {
        const int targetDirectionDegrees = rtbMapObject->targetDirectionDegrees();
        rtbMapObject->setTargetDirectionDegrees(mIntValue);
        mIntValue = targetDirectionDegrees;
        break;
    }
    case RTBIntervalOffset: {
        if(rtbMapObject->beamType() != RTBMapObject::BT2)
            return;

        const int intervalOffset = rtbMapObject->intervalOffset();
        rtbMapObject->setIntervalOffset(mIntValue);
        mIntValue = intervalOffset;
        break;
    }
    case RTBIntervalSpeed: {
        const int intervalSpeed = rtbMapObject->intervalSpeed();
        rtbMapObject->setIntervalSpeed(mIntValue);
        mIntValue = intervalSpeed;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBProjectileTurret *rtbMapObject)
{
    switch (mProperty) {
    case RTBIntervalSpeed: {
        const int intervalSpeed = rtbMapObject->convertToIndex(RTBMapObject::IntervalSpeed
                                                               , rtbMapObject->intervalSpeed());
        rtbMapObject->setIntervalSpeed(rtbMapObject->convertToID(RTBMapObject::IntervalSpeed, mIntValue));
        mIntValue = intervalSpeed;
        break;
    }
    case RTBIntervalOffset: {
        const int intervalOffset = rtbMapObject->convertToIndex(RTBMapObject::IntervalOffset
                                                                , rtbMapObject->intervalOffset());
        rtbMapObject->setIntervalOffset(rtbMapObject->convertToID(RTBMapObject::IntervalOffset, mIntValue));
        mIntValue = intervalOffset;
        break;
    }
    case RTBProjectileSpeed: {
        const int projectileSpeed = rtbMapObject->projectileSpeed();
        rtbMapObject->setProjectileSpeed(mIntValue);
        mIntValue = projectileSpeed;
        break;
    }
    case RTBShotDirection: {
        const int shotDirection = rtbMapObject->shotDirection();
        rtbMapObject->setShotDirection(mIntValue);
        mIntValue = shotDirection;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBTeleporter *rtbMapObject)
{
    switch (mProperty) {
    case RTBTeleporterTarget: {
        const QString target = rtbMapObject->teleporterTarget();
        rtbMapObject->setTeleporterTarget(mStringValue);
        mStringValue = target;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBFloorText *rtbMapObject)
{
    switch (mProperty) {
    case RTBText: {
        const QString text = rtbMapObject->text();
        rtbMapObject->setText(mStringValue);
        mStringValue = text;
        break;
    }
    case RTBMaxCharacters: {
        const int maxCharacters = rtbMapObject->maxCharacters();
        rtbMapObject->setMaxCharacters(mIntValue);
        mIntValue = maxCharacters;
        break;
    }
    case RTBTriggerZone: {
        const QSizeF size = rtbMapObject->triggerZoneSize();
        rtbMapObject->setTriggerZoneSize(mSizeValue);
        mSizeValue = size;
        break;
    }
    case RTBUseTrigger: {
        const int useTrigger = rtbMapObject->useTrigger();
        rtbMapObject->setUseTrigger(mIntValue);
        mIntValue = useTrigger;
        break;
    }
    case RTBOffsetX: {
        const double offsetX = rtbMapObject->offsetX();
        rtbMapObject->setOffsetX(mDoubleValue);
        mDoubleValue = offsetX;
        break;
    }
    case RTBOffsetY: {
        const double offsetY = rtbMapObject->offsetY();
        rtbMapObject->setOffsetY(mDoubleValue);
        mDoubleValue = offsetY;
        break;
    }
    case RTBScale: {
        const double scale = rtbMapObject->scale();
        rtbMapObject->setScale(mDoubleValue);
        mDoubleValue = scale;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBCameraTrigger *rtbMapObject)
{
    switch (mProperty) {
    case RTBCameraTarget: {
        const QString target = rtbMapObject->target();
        rtbMapObject->setTarget(mStringValue);
        mStringValue = target;
        break;
    }
    case RTBTriggerZone: {
        const QSizeF size = rtbMapObject->triggerZoneSize();
        rtbMapObject->setTriggerZoneSize(mSizeValue);
        mSizeValue = size;
        break;
    }
    case RTBCameraHeight: {
        const int cameraHeight = rtbMapObject->cameraHeight();
        rtbMapObject->setCameraHeight(mIntValue);
        mIntValue = cameraHeight;
        break;
    }
    case RTBCameraAngle: {
        const int cameraAngle = rtbMapObject->cameraAngle();
        rtbMapObject->setCameraAngle(mIntValue);
        mIntValue = cameraAngle;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::swap(RTBNPCBallSpawner *rtbMapObject)
{
    switch (mProperty) {
    case RTBSpawnClass: {
        const int spawnClass = rtbMapObject->spawnClass();
        rtbMapObject->setSpawnClass(mIntValue);
        mIntValue = spawnClass;
        break;
    }
    case RTBSize: {
        const int size = rtbMapObject->size();
        rtbMapObject->setSize(mIntValue);
        mIntValue = size;
        break;
    }
    case RTBIntervalOffset: {
        const int intervalOffset = rtbMapObject->convertToIndex(RTBMapObject::IntervalOffset
                                                                , rtbMapObject->intervalOffset());
        rtbMapObject->setIntervalOffset(rtbMapObject->convertToID(RTBMapObject::IntervalOffset, mIntValue));
        mIntValue = intervalOffset;
        break;
    }
    case RTBSpawnFrequency: {
        const int spawnFrequency = rtbMapObject->spawnFrequency();
        rtbMapObject->setSpawnFrequency(mIntValue);
        mIntValue = spawnFrequency;
        break;
    }
    case RTBSpeed: {
        const int speed = rtbMapObject->speed();
        rtbMapObject->setSpeed(mIntValue);
        mIntValue = speed;
        break;
    }
    case RTBShotDirection: {
        const int direction = rtbMapObject->direction();
        rtbMapObject->setDirection(mIntValue);
        mIntValue = direction;
        break;
    }
    }
}

void RTBChangeMapObjectProperties::removeLaserFromRelatedButton()
{
    for(MapObject *obj: mMapDocument->map()->objectGroups().first()->objects())
    {
        if(obj->rtbMapObject()->objectType() == RTBMapObject::Button)
        {
            RTBButtonObject *buttonObject = static_cast<RTBButtonObject*>(obj->rtbMapObject());

            // check if one of the targets points to this laser, if so delete entry
            if(mMapObject->id() == buttonObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget1).toInt())
            {
                buttonObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget1, QString());
                mRelatedButtons.append(obj);
                break;
            }
            else if(mMapObject->id() == buttonObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget2).toInt())
            {
                buttonObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget2, QString());
                mRelatedButtons.append(obj);
                break;
            }
            else if(mMapObject->id() == buttonObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget3).toInt())
            {
                buttonObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget3, QString());
                mRelatedButtons.append(obj);
                break;
            }
            else if(mMapObject->id() == buttonObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget4).toInt())
            {
                buttonObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget4, QString());
                mRelatedButtons.append(obj);
                break;
            }
            else if(mMapObject->id() == buttonObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget5).toInt())
            {
                buttonObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget5, QString());
                mRelatedButtons.append(obj);
                break;
            }

            // update property view
            mMapDocument->mapObjectModel()->emitObjectsChanged(mRelatedButtons);
        }
    }
}

void RTBChangeMapObjectProperties::addLaserToRelatedButton()
{
    if(mRelatedButtons.size() > 0)
    {
        RTBButtonObject *button = static_cast<RTBButtonObject*>(mRelatedButtons.first()->rtbMapObject());

        // search for a free slot and insert this laser as target
        if(QString() == button->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget1))
        {
            button->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget1,  QString::number(mMapObject->id()));
        }
        else if(QString() == button->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget2))
        {
            button->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget2,  QString::number(mMapObject->id()));
        }
        else if(QString() == button->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget3))
        {
            button->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget3,  QString::number(mMapObject->id()));
        }
        else if(QString() == button->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget4))
        {
            button->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget4,  QString::number(mMapObject->id()));
        }
        else if(QString() == button->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget5))
        {
            button->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget5,  QString::number(mMapObject->id()));
        }

        // update property view
        mMapDocument->mapObjectModel()->emitObjectsChanged(mRelatedButtons);
    }
}
