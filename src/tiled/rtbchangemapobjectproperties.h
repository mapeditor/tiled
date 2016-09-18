/*
 * rtbchangemapobjectproperties.h
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

#ifndef RTBCHANGEMAPOBJECTPROPERTIES_H
#define RTBCHANGEMAPOBJECTPROPERTIES_H

#include "mapobject.h"
#include "mapdocument.h"

#include <QColor>
#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class RTBChangeMapObjectProperties : public QUndoCommand
{
public:
    enum Property {
        RTBIntervalSpeed,
        RTBIntervalOffset,
        RTBSpawnAmount,
        RTBBeatsActive,
        RTBLaserBeamTargets,
        RTBBeamType,
        RTBActivatedOnStart,
        RTBDirectionDegrees,
        RTBTargetDirectionDegrees,
        RTBProjectileSpeed,
        RTBShotDirection,
        RTBTeleporterTarget,
        RTBLaserBeamTarget1,
        RTBLaserBeamTarget2,
        RTBLaserBeamTarget3,
        RTBLaserBeamTarget4,
        RTBLaserBeamTarget5,
        RTBText,
        RTBMaxCharacters,
        RTBTriggerZone,
        RTBUseTrigger,
        RTBCameraTarget,
        RTBCameraHeight,
        RTBCameraAngle,
        RTBRandomizeStart,
        RTBScale,
        RTBSpawnClass,
        RTBSize,
        RTBSpawnFrequency,
        RTBSpeed,
        RTBOffsetX,
        RTBOffsetY
    };

    /**
     * Constructs a command that changes the value of the given property.
     *
     *
     * @param mapObject         the map object of the map
     *
     */
    RTBChangeMapObjectProperties(MapDocument *mapDocument, MapObject *mapObject, Property property, int value);
    RTBChangeMapObjectProperties(MapDocument *mapDocument, MapObject *mapObject, Property property, QString value);
    RTBChangeMapObjectProperties(MapDocument *mapDocument, MapObject *mapObject, Property property, QSizeF value);
    RTBChangeMapObjectProperties(MapDocument *mapDocument, MapObject *mapObject, Property property, double value);

    void undo();
    void redo();

private:
    void swap();

    void swap(RTBCustomFloorTrap *rtbMapObject);
    void swap(RTBMovingFloorTrapSpawner *rtbMapObject);
    void swap(RTBButtonObject *rtbMapObject);
    void swap(RTBLaserBeam *rtbMapObject);
    void swap(RTBProjectileTurret *rtbMapObject);
    void swap(RTBTeleporter *rtbMapObject);
    void swap(RTBFloorText *rtbMapObject);
    void swap(RTBCameraTrigger *rtbMapObject);
    void swap(RTBNPCBallSpawner *rtbMapObject);

    void removeLaserFromRelatedButton();
    void addLaserToRelatedButton();

    MapDocument *mMapDocument;
    MapObject *mMapObject;
    Property mProperty;
    QString mStringValue;
    QSizeF mSizeValue;

    QList<MapObject*> mRelatedButtons;

    union {
        int mIntValue;
        double mDoubleValue;
    };

};

}
}
#endif // RTBCHANGEMAPOBJECTPROPERTIES_H
