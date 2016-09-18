/*
 * rtbmapobject.h
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

#ifndef RTBMAPOBJECT_H
#define RTBMAPOBJECT_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector2D>
#include <QHash>
#include <QColor>
#include <QSizeF>
#include <QVariant>
#include <QCoreApplication>

namespace Tiled {

static const char* CUSTOM_FLOOR_TRAP = "Custom Floor Trap";
static const char* CUSTOM_FLOOR_TRAP_SPAWNER = "Moving Floor Trap Spawner";
static const char* BUTTON = "Button";
static const char* LASER_BEAM_LEFT = "Laser Beam Left";
static const char* LASER_BEAM_BOTTOM = "Laser Beam Bottom";
static const char* LASER_BEAM_TOP = "Laser Beam Top";
static const char* LASER_BEAM_RIGHT = "Laser Beam Right";
static const char* PROJECTILE_TURRET = "Projectile Turret";
static const char* TELEPORTER = "Teleporter";
static const char* TARGET = "Target";
static const char* LASER_BEAM = "Laser Beam";
static const char* FLOOR_TEXT = "Floor Text";
static const char* CAMERA_TRIGGER = "Camera Trigger";
static const char* START_LOCATION = "Start Location";
static const char* FINISH_HOLE = "Finish Hole";
static const char* WARBALL_SPAWNER = "Warball Spawner";
static const char* POINT_ORB = "Point Orb";
static const char* CHECKPOINT_ORB = "Checkpoint Orb";
static const char* HEALTH_ORB = "Health Orb";
static const char* KEY_ORB = "Key Orb";
static const char* FAKE_ORB = "Fake Orb";
static const char* ORB = "Orb";
static const char* OBJECT = "Object";

class RTBMapObject
{
protected:
    // struct with the default values
    struct ObjectDefaults
    {
        ObjectDefaults()
        {
        }

        QVariant defaultValue(int id) const
        {
            if(mObjectDefaults.contains(id))
                return mObjectDefaults.value(id);

            return QVariant();
        }

    protected:
        QHash<int,QVariant> mObjectDefaults;

        virtual QHash<int,QVariant> initObjectDefaults() = 0;
    };

public:
    RTBMapObject();

    enum Objects {
        PointOrb = 24,
        CheckpointOrb = 25,
        HealthOrb = 26,
        KeyOrb = 27,
        FakeOrb = 28,
        CustomFloorTrap = 40,
        MovingFloorTrapSpawner = 41,
        Button = 42,
        LaserBeamLeft = 44,
        LaserBeamBottom = 45,
        LaserBeamTop = 46,
        LaserBeamRight = 47,
        ProjectileTurret = 48,
        Teleporter = 49,
        Target = 50,
        FloorText = 51,
        CameraTrigger = 52,
        StartLocation = 53,
        FinishHole = 54,
        NPCBallSpawner = 55,
        LaserBeam = 60,
        Orb = 64
    };

    enum BeamType {
        BT0,
        BT1,
        BT2
    };

    enum SpawnClass {
        SC0,    // ROLLING
        SC1     // DROPPING
    };

    enum Size {
        SMALL,
        NORMAL,
        LARGE
    };

    enum LaserBeamTargets {
        Target1 = 14,
        Target2 = 15,
        Target3 = 16,
        Target4 = 17,
        Target5 = 18
    };

    enum Direction {
        Left,
        Right,
        Up,
        Down,
        All
    };

    enum ErrorState {
        Nothing,
        Warning,
        Error
    };

    enum PropertyId {
        IntervalSpeed = 100,
        IntervalOffset,
        SpawnAmount,
        BeatsActive,
        LaserBeamTargets,
        BeamType,
        ActivatedOnStart,
        DirectionDegrees,
        TargetDirectionDegrees,
        ProjectileSpeed,
        ShotDirection,
        TeleporterTarget,
        LaserBeamTarget1,
        LaserBeamTarget2,
        LaserBeamTarget3,
        LaserBeamTarget4,
        LaserBeamTarget5,
        Text,
        MaxCharacter,
        TriggerZone,
        UseTrigger,
        CameraTarget,
        CameraHeight,
        CameraAngle,
        RandomizeStart,
        Scale,
        SpawnClass,
        Size,
        SpawnFrequency,
        Speed,
        OffsetX,
        OffsetY
    };

    static QString objectName(int id)
    {
        switch (id) {
        case CustomFloorTrap:
            return QCoreApplication::translate("Tiled::RTBMapObject", CUSTOM_FLOOR_TRAP);
            break;
        case MovingFloorTrapSpawner:
            return QCoreApplication::translate("Tiled::RTBMapObject", CUSTOM_FLOOR_TRAP_SPAWNER);
            break;
        case Button:
            return QCoreApplication::translate("Tiled::RTBMapObject", BUTTON);
            break;
        case LaserBeamLeft:
            return QCoreApplication::translate("Tiled::RTBMapObject", LASER_BEAM_LEFT);
            break;
        case LaserBeamBottom:
            return QCoreApplication::translate("Tiled::RTBMapObject", LASER_BEAM_BOTTOM);
            break;
        case LaserBeamTop:
            return QCoreApplication::translate("Tiled::RTBMapObject", LASER_BEAM_TOP);
            break;
        case LaserBeamRight:
            return QCoreApplication::translate("Tiled::RTBMapObject", LASER_BEAM_RIGHT);
            break;
        case ProjectileTurret:
            return QCoreApplication::translate("Tiled::RTBMapObject", PROJECTILE_TURRET);
            break;
        case Teleporter:
            return QCoreApplication::translate("Tiled::RTBMapObject", TELEPORTER);
            break;
        case Target:
            return QCoreApplication::translate("Tiled::RTBMapObject", TARGET);
            break;
        case LaserBeam:
            return QCoreApplication::translate("Tiled::RTBMapObject", LASER_BEAM);
            break;
        case FloorText:
            return QCoreApplication::translate("Tiled::RTBMapObject", FLOOR_TEXT);
            break;
        case CameraTrigger:
            return QCoreApplication::translate("Tiled::RTBMapObject", CAMERA_TRIGGER);
            break;
        case StartLocation:
            return QCoreApplication::translate("Tiled::RTBMapObject", START_LOCATION);
            break;
        case FinishHole:
            return QCoreApplication::translate("Tiled::RTBMapObject", FINISH_HOLE);
            break;
        case NPCBallSpawner:
            return QCoreApplication::translate("Tiled::RTBMapObject", WARBALL_SPAWNER);
            break;

        // ORBS
        case RTBMapObject::PointOrb:
            return QCoreApplication::translate("Tiled::RTBMapObject", POINT_ORB);
            break;
        case RTBMapObject::CheckpointOrb:
            return QCoreApplication::translate("Tiled::RTBMapObject", CHECKPOINT_ORB);
            break;
        case RTBMapObject::HealthOrb:
            return QCoreApplication::translate("Tiled::RTBMapObject", HEALTH_ORB);
            break;
        case RTBMapObject::KeyOrb:
            return QCoreApplication::translate("Tiled::RTBMapObject", KEY_ORB);
            break;
        case RTBMapObject::FakeOrb:
            return QCoreApplication::translate("Tiled::RTBMapObject", FAKE_ORB);
            break;
        case RTBMapObject::Orb:
            return QCoreApplication::translate("Tiled::RTBMapObject", ORB);
            break;
        default:
            return QCoreApplication::translate("Tiled::RTBMapObject", OBJECT);
            break;
        }
    }

    virtual RTBMapObject *clone() const = 0;

    QString name()
    {
        return objectName(mObjectType);
    }

    /**
     * Returns the type of this object.
     */
    int objectType() const { return mObjectType; }

    /**
     * Sets the type of this object.
     */
    void setObjectType(int value) {  mObjectType = value; }

    /**
     * Returns the hasError state
     */
    bool hasError() const { return mHasError; }

    /**
     * Sets the hasError state
     */
    void setHasError(bool hasError)
    {
        if(!mHasError)
            mHasError = hasError;
    }

    /**
     * Returns the hasWarning state
     */
    int hasWarning() const { return mHasWarning; }

    /**
     * Sets the hasWarning state
     */
    void setHasWarning(bool hasWarning)
    {
        if(!mHasWarning)
            mHasWarning = hasWarning;
    }

    void setPropertyErrorState(PropertyId id, ErrorState state)
    {
        if(state == Error)
        {
            mPropertyErrorState[id] = state;
            mHasError = true;
        }
        else if(state == Warning && mPropertyErrorState[id] != Error)
        {
            mPropertyErrorState[id] = state;
            mHasWarning = true;
        }
    }

    int propertyErrorState(PropertyId id) const { return mPropertyErrorState[id]; }

    void clearErrorState()
    {
        clearPropertyErrorState();

        mHasError = false;
        mHasWarning = false;
    }

    QColor nameColor(PropertyId id) const
    {
        switch (mPropertyErrorState[id]) {
        case Error:
            return Qt::red;
            break;
        case Warning:
            return warningColor();
            break;
        case Nothing:
        default:
            return Qt::black;
            break;
        }
    }

    static QColor warningColor()
    {
        return QColor("#ffbf00");
    }

    /**
     * Returns the id of the origin object, if this is a cloned object
     */
    int originID() const { return mOriginID; }

    /**
     * Sets the origin id of this cloned object
     */
    void setOriginID(int value) {  mOriginID = value; }

    virtual QVariant defaultValue(int id) = 0;

private:
    void clearPropertyErrorState()
    {
        QHash<PropertyId, int>::iterator i;
        for (i = mPropertyErrorState.begin(); i != mPropertyErrorState.end(); ++i)
            i.value() = 0;
    }

    int mObjectType;
    bool mHasError;
    bool mHasWarning;
    QHash<PropertyId, int> mPropertyErrorState;
    int mOriginID;
};

//=============================================================================

class RTBButtonObject : public RTBMapObject
{
private:
    // struct with the default values
    struct ButtonDefaults : public ObjectDefaults
    {
        ButtonDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(BeatsActive, 0);
            objectDefaults.insert(LaserBeamTargets, QLatin1String(""));
            objectDefaults.insert(LaserBeamTarget1, 0);
            objectDefaults.insert(LaserBeamTarget2, 0);
            objectDefaults.insert(LaserBeamTarget3, 0);
            objectDefaults.insert(LaserBeamTarget4, 0);
            objectDefaults.insert(LaserBeamTarget5, 0);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBButtonObject();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    int beatsActive() const { return mBeatsActive; }
    void setBeatsActive(int value) {  mBeatsActive = value; }

    QString laserBeamTargets() const { return mLaserBeamTargets; }
    void setLaserBeamTargets(QString value)
    {
        mLaserBeamTargets = value;

        // store the single target ids
        QStringList values = value.split(QString::fromStdString(", "));
        for(int i = 0; i < values.size(); i++)
        {
             mTargerts.insert(Target1 + i, values.at(i));
        }
    }

    void insertTarget(int targetNumber, QString targetID)
    {
        mTargerts.insert(targetNumber, targetID);
        mLaserBeamTargets = targets();
    }

    void removeTarget(int targetNumber) { mTargerts.remove(targetNumber); }

    QString target(int targetNumber) const { return mTargerts.value(targetNumber); }

    int targetCount() { return mTargerts.size(); }

    QString targets()
    {
        QString targets;
        QMap<int, QString>::iterator i;
        for (i = mTargerts.begin(); i != mTargerts.end(); ++i)
        {
            if(i.value() != QString::fromStdString(""))
                targets += i.value() + QString::fromStdString(", ");
        }

        return targets.left(targets.size()-2);
    }

    bool containsTarget(QString targetID)
    {
        for(QString id : mTargerts)
        {
            if(id == targetID)
                return true;
        }
        return false;
    }

    int findTargetNumber(QString targetID)
    {
        if(target(Target1) == targetID)
            return Target1;
        if(target(Target2) == targetID)
            return Target2;
        if(target(Target3) == targetID)
            return Target3;
        if(target(Target4) == targetID)
            return Target4;
        if(target(Target5) == targetID)
            return Target5;

        return 0;
    }

    bool appendTarget(QString targetID)
    {
        for(int i = Target1; i <= Target5; i++)
        {
            if(target(i).isEmpty())
            {
                insertTarget(i, targetID);
                return true;
            }
        }

        return false;
    }

private:
    int mBeatsActive;
    QString mLaserBeamTargets;
    QMap<int, QString> mTargerts;
    int mTargetCount;

};

//=============================================================================

class RTBCustomFloorTrap : public RTBMapObject
{
private:
    // struct with the default values
    struct CustomFloorTrapDefaults : public ObjectDefaults
    {
        CustomFloorTrapDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(IntervalSpeed, 1);
            objectDefaults.insert(IntervalOffset, 0);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBCustomFloorTrap();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    int intervalSpeed() const { return mIntervalSpeed; }
    void setIntervalSpeed(int value) {  mIntervalSpeed = value; }

    int intervalOffset() const { return mIntervalOffset; }
    void setIntervalOffset(int value) {  mIntervalOffset = value; }

private:
    int mIntervalSpeed;
    int mIntervalOffset;

};

//=============================================================================

class RTBMovingFloorTrapSpawner : public RTBMapObject
{
private:
    // struct with the default values
    struct MovingFloorTrapSpawnerDefaults : public ObjectDefaults
    {
        MovingFloorTrapSpawnerDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(SpawnAmount, 1);
            objectDefaults.insert(IntervalSpeed, 1);
            objectDefaults.insert(RandomizeStart, true);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBMovingFloorTrapSpawner();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    int spawnAmount() const { return mSpawnAmount; }
    void setSpawnAmount(int value) {  mSpawnAmount = value; }

    int intervalSpeed() const { return mIntervalSpeed; }
    void setIntervalSpeed(int value) {  mIntervalSpeed = value; }

    bool randomizeStart() const { return mRandomizeStart; }
    void setRandomizeStart(bool value) {  mRandomizeStart = value; }

private:
    int mSpawnAmount;
    int mIntervalSpeed;
    bool mRandomizeStart;

};

//=============================================================================

// IMPORTANT: postition  represent the position in the dropdown
//const QList<int> projectileTurretIS({1, 2, 3, 4});
//const QList<int> projectileTurretIO({0, 8});
const QList<int> projectileTurretIS = QList<int>() << 1 << 2 << 3 << 4;
const QList<int> projectileTurretIO = QList<int>() << 0 << 8;

class RTBProjectileTurret : public RTBMapObject
{
private:
    // struct with the default values
    struct ProjectileTurretDefaults : public ObjectDefaults
    {
        ProjectileTurretDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(IntervalSpeed, 0);
            objectDefaults.insert(IntervalOffset, 0);
            objectDefaults.insert(ProjectileSpeed, 300);
            objectDefaults.insert(ShotDirection, 0);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBProjectileTurret();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    int intervalSpeed() const { return mIntervalSpeed; }
    void setIntervalSpeed(int intervalSpeed)
    {
        mIntervalSpeed = idExists(RTBMapObject::IntervalSpeed, intervalSpeed);
    }

    int intervalOffset() const { return mIntervalOffset; }
    void setIntervalOffset(int intervalOffset)
    {
        mIntervalOffset = idExists(RTBMapObject::IntervalOffset, intervalOffset);
    }

    int projectileSpeed() const { return mProjectileSpeed; }
    void setProjectileSpeed(int value) {  mProjectileSpeed = value; }

    int shotDirection() const { return mShotDirection; }
    void setShotDirection(int value) {  mShotDirection = value; }

    // if given id exists return it else return the first id in list
    int idExists(PropertyId property, int id)
    {
        if(property == IntervalSpeed)
        {
            if(projectileTurretIS.contains(id))
                return id;
            else
                return projectileTurretIS.first();
        }
        else if(property == IntervalOffset)
        {
            if(projectileTurretIO.contains(id))
                return id;
            else
                return projectileTurretIO.first();
        }

        return 0;
    }

    int convertToIndex(PropertyId property, int id) const
    {
        int index;
        if(property == IntervalSpeed)
        {
            index = projectileTurretIS.indexOf(id);
        }
        else if(property == IntervalOffset)
        {
            index = projectileTurretIO.indexOf(id);
        }

        if(index >= 0)
            return index;
        else
            return 0;
    }

    int convertToID(PropertyId property, int index) const
    {
        int id;
        if(property == IntervalSpeed)
        {
            id = projectileTurretIS.at(index);
        }
        else if(property == IntervalOffset)
        {
            id = projectileTurretIO.at(index);
        }

        if(id >= 0)
            return id;
        else
            return 0;
    }

    int idCount(PropertyId property) const
    {
        if(property == IntervalSpeed)
            return projectileTurretIS.size();
        else if(property == IntervalOffset)
            return projectileTurretIO.size();

        return 0;
    }

private:
    int mIntervalSpeed;
    int mIntervalOffset;
    int mProjectileSpeed;
    int mShotDirection;

};

//=============================================================================

class RTBTeleporter : public RTBMapObject
{   
private:
    // struct with the default values
    struct TeleporterDefaults : public ObjectDefaults
    {
        TeleporterDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(TeleporterTarget, QLatin1String(""));

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBTeleporter();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    QString teleporterTarget() const { return mTeleporterTarget; }
    void setTeleporterTarget(QString value) {  mTeleporterTarget = value; }

private:
    QString mTeleporterTarget;

};

//=============================================================================

class RTBLaserBeam : public RTBMapObject
{
private:
    // struct with the default values
    struct LaserBeamDefaults : public ObjectDefaults
    {
        LaserBeamDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(BeamType, 0);
            objectDefaults.insert(ActivatedOnStart, true);
            objectDefaults.insert(DirectionDegrees, 0);
            objectDefaults.insert(TargetDirectionDegrees, 0);
            objectDefaults.insert(IntervalOffset, 0);
            objectDefaults.insert(IntervalSpeed, 1);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBLaserBeam();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    int beamType() const { return mBeamType; }
    void setBeamType(int value) {  mBeamType = value; }

    bool activatedOnStart() const { return mActivatedOnStart; }
    void setActivatedOnStart(bool value) {  mActivatedOnStart = value; }

    int directionDegrees() const { return mDirectionDegrees; }
    void setDirectionDegrees(int value) {  mDirectionDegrees = value; }

    int targetDirectionDegrees() const { return mTargetDirectionDegrees; }
    void setTargetDirectionDegrees(int value) {  mTargetDirectionDegrees = value; }

    int intervalOffset() const { return mIntervalOffset; }
    void setIntervalOffset(int value) {  mIntervalOffset = value; }

    int intervalSpeed() const { return mIntervalSpeed; }
    void setIntervalSpeed(int value) {  mIntervalSpeed = value; }

private:
    int mBeamType;
    bool mActivatedOnStart;
    int mDirectionDegrees;
    int mTargetDirectionDegrees;
    int mIntervalOffset;
    int mIntervalSpeed;

};

//=============================================================================

class RTBTarget : public RTBMapObject
{
public:
    RTBTarget();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        Q_UNUSED(id);

        //return mDefaults.defaultValue(id);
        return QVariant();
    }
};

//=============================================================================

class RTBFloorText : public RTBMapObject
{
private:
    // struct with the default values
    struct FloorTextDefaults : public ObjectDefaults
    {
        FloorTextDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(Text, QLatin1String(""));
            objectDefaults.insert(MaxCharacter, 50);
            objectDefaults.insert(TriggerZone, QSizeF(1, 1));
            objectDefaults.insert(UseTrigger, false);
            objectDefaults.insert(Scale, 1);
            objectDefaults.insert(OffsetX, 0);
            objectDefaults.insert(OffsetY, 0);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBFloorText();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    QString text() const { return mText; }
    void setText(QString text) { mText = text; }

    int maxCharacters() const { return mMaxCharacters; }
    void setMaxCharacters(int maxCharacter) { mMaxCharacters = maxCharacter; }

    QSizeF triggerZoneSize() const { return mTriggerZoneSize; }
    void setTriggerZoneSize(QSizeF triggerZoneSize) {  mTriggerZoneSize = triggerZoneSize; }

    bool useTrigger() const { return mUseTrigger; }
    void setUseTrigger(bool useTrigger) { mUseTrigger = useTrigger; }

    double scale() const { return mScale; }
    void setScale(double scale){  mScale = scale; }

    double offsetX() const { return mOffsetX; }
    void setOffsetX(double offsetX) { mOffsetX = offsetX; }

    double offsetY() const { return mOffsetY; }
    void setOffsetY(double offsetY) { mOffsetY = offsetY; }

private:
    QString mText;
    int mMaxCharacters;
    QSizeF mTriggerZoneSize;
    bool mUseTrigger;
    double mScale;
    double mOffsetX;
    double mOffsetY;
};

//=============================================================================

class RTBCameraTrigger : public RTBMapObject
{
private:
    // struct with the default values
    struct CameraTriggerDefaults : public ObjectDefaults
    {
        CameraTriggerDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(CameraTarget, QLatin1String(""));
            objectDefaults.insert(TriggerZone, QSizeF(1, 1));
            objectDefaults.insert(CameraHeight, 1500);
            objectDefaults.insert(CameraAngle, 75);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBCameraTrigger();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    QString target() const { return mTarget; }
    void setTarget(QString target) {  mTarget = target; }

    QSizeF triggerZoneSize() const { return mTriggerZoneSize; }
    void setTriggerZoneSize(QSizeF triggerZoneSize) {  mTriggerZoneSize = triggerZoneSize; }

    int cameraHeight() const { return mCameraHeight; }
    void setCameraHeight(int cameraHeight) {  mCameraHeight = cameraHeight; }

    int cameraAngle() const { return mCameraAngle; }
    void setCameraAngle(int cameraAngle) {  mCameraAngle = cameraAngle; }

private:
    QString mTarget;
    QSizeF mTriggerZoneSize;
    int mCameraHeight;
    int mCameraAngle;
};

//=============================================================================

class RTBStartLocation : public RTBMapObject
{
public:
    RTBStartLocation();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        Q_UNUSED(id);

        //return mDefaults.defaultValue(id);
        return QVariant();
    }
};

//=============================================================================

class RTBFinishHole : public RTBMapObject
{
public:
    RTBFinishHole();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        Q_UNUSED(id);

        //return mDefaults.defaultValue(id);
        return QVariant();
    }
};

//=============================================================================

class RTBOrb : public RTBMapObject
{
public:
    RTBOrb();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        Q_UNUSED(id);

        //return mDefaults.defaultValue(id);
        return QVariant();
    }
};

//=============================================================================

// IMPORTANT: postition  represent the position in the dropdown
const QList<int> npcBallSpawnerIO = QList<int>() << 0 << 8;

class RTBNPCBallSpawner : public RTBMapObject
{
private:
    // struct with the default values
    struct NPCBallSpawnerDefaults : public ObjectDefaults
    {
        NPCBallSpawnerDefaults()
            : ObjectDefaults()
        {
            mObjectDefaults = initObjectDefaults();
        }

    private:
        QHash<int,QVariant> initObjectDefaults()
        {
            QHash<int,QVariant> objectDefaults;
            objectDefaults.insert(SpawnClass, 0);
            objectDefaults.insert(Size, 1);
            objectDefaults.insert(IntervalOffset, 0);
            objectDefaults.insert(SpawnFrequency, 0);
            objectDefaults.insert(Speed, 150);
            objectDefaults.insert(ShotDirection, 1);

            return objectDefaults;
        }
    } const mDefaults;

public:
    RTBNPCBallSpawner();

    RTBMapObject *clone() const;

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }

    int spawnClass() const { return mSpawnClass; }
    void setSpawnClass(int spawnClass) {  mSpawnClass = spawnClass; }

    int size() const { return mSize; }
    void setSize(int size) {  mSize = size; }

    int intervalOffset() const { return mIntervalOffset; }
    void setIntervalOffset(int intervalOffset)
    {
        mIntervalOffset = idExists(RTBMapObject::IntervalOffset, intervalOffset);
    }

    int spawnFrequency() const { return mSpawnFrequency; }
    void setSpawnFrequency(int spawnFrequency) {  mSpawnFrequency = spawnFrequency; }

    int speed() const { return mSpeed; }
    void setSpeed(int speed) {  mSpeed = speed; }

    int direction() const { return mDirection; }
    void setDirection(int direction) {  mDirection = direction; }

    // if given id exists return it else return the first id in list
    int idExists(PropertyId property, int id)
    {
        if(property == IntervalOffset)
        {
            if(npcBallSpawnerIO.contains(id))
                return id;
            else
                return npcBallSpawnerIO.first();
        }

        return 0;
    }

    int convertToIndex(PropertyId property, int id) const
    {
        int index;
        if(property == IntervalOffset)
        {
            index = npcBallSpawnerIO.indexOf(id);
        }

        if(index >= 0)
            return index;
        else
            return 0;
    }

    int convertToID(PropertyId property, int index) const
    {
        int id;
        if(property == IntervalOffset)
        {
            id = npcBallSpawnerIO.at(index);
        }

        if(id >= 0)
            return id;
        else
            return 0;
    }

    int idCount(PropertyId property) const
    {
        if(property == IntervalOffset)
            return projectileTurretIO.size();

        return 0;
    }

private:
    int mSpawnClass;
    int mSize;
    int mIntervalOffset;
    int mSpawnFrequency;
    int mSpeed;
    int mDirection;
};

}

#endif // RTBMAPOBJECT_H
