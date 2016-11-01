/*
 * rtbcore.h
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

#ifndef RTBCORE_H
#define RTBCORE_H

#include "rtbmapsettings.h"

#include <QDir>
#include <QObject>
#include <QString>

namespace Tiled {
namespace Internal {

class MapObjectItem;

static const char* FLOOR = "Floor";
static const char* FLOOR_TRAP = "Floor Trap";
static const char* BARRIER = "Barrier";
static const char* HIDDEN_FLOOR = "Hidden Floor";
static const char* WALL_BLOCK = "Wall Block";
static const char* SPEEDPAD_RIGHT = "Speedpad Right";
static const char* SPEEDPAD_LEFT = "Speedpad Left";
static const char* SPEEDPAD_UP = "Speedpad Up";
static const char* SPEEDPAD_DOWN = "Speedpad Down";
static const char* JUMPPAD = "Jumppad";
static const char* TILE = "Tile";

class RTBCore : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns the core instance. Creates the instance when it
     * doesn't exist yet.
     */
    static RTBCore *instance();

    /**
     * Deletes the corer instance if it exists.
     */
    static void deleteInstance();


    static QString gameExe() { return mGameExe; }
    static QString gameShippingExe() { return mGameShippingExe; }

    QString findGameDirectory();

    bool isGameAlreadyRunning();
    void buildMap(MapDocument *mapDocument);

    bool isHalfTileAllowed(MapObject *mapObject);
    bool isHalfTileAllowed(QSet<Tiled::Internal::MapObjectItem *> mapObjectItems);

    static QString tileName(int id)
    {
        switch (id) {
        case RTBMapSettings::Floor:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", FLOOR);
            break;
        case RTBMapSettings::FloorTrap:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", FLOOR_TRAP);
            break;
        case RTBMapSettings::Barrier:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", BARRIER);
            break;
        case RTBMapSettings::HiddenFloor:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", HIDDEN_FLOOR);
            break;
        case RTBMapSettings::WallBlock:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", WALL_BLOCK);
            break;
        case RTBMapSettings::SpeedpadRight:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", SPEEDPAD_RIGHT);
            break;
        case RTBMapSettings::SpeedpadLeft:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", SPEEDPAD_LEFT);
            break;
        case RTBMapSettings::SpeedpadUp:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", SPEEDPAD_UP);
            break;
        case RTBMapSettings::SpeedpadDown:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", SPEEDPAD_DOWN);
            break;
        case RTBMapSettings::Jumppad:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", JUMPPAD);
            break;
        default:
            return QCoreApplication::translate("Tiled::Internal::RTBCore", TILE);
            break;
        }
    }

    static QString intervallOffsetValue(RTBMapObject::Objects objectType, int id)
    {
        if(objectType == RTBMapObject::ProjectileTurret
                || objectType == RTBMapObject::NPCBallSpawner)
        {
            switch (id) {
            case 0:
                return QLatin1String("0");
            case 8:
                return QLatin1String("1/2");
            default:
                return QLatin1String("0");
            }
        }
        else
        {
            switch (id) {
            case 0:
                return QLatin1String("0");
            case 1:
                return QLatin1String("1/8");
            case 2:
                return QLatin1String("2/8");
            case 3:
                return QLatin1String("3/8");
            case 4:
                return QLatin1String("4/8");
            case 5:
                return QLatin1String("5/8");
            case 6:
                return QLatin1String("6/8");
            case 7:
                return QLatin1String("7/8");
            default:
                return QLatin1String("0");
            }
        }
    }

private:
    RTBCore();

    QString createPath(QDir dir);

    static RTBCore *mInstance;

    static const QString mGameExe;
    static const QString mGameShippingExe;

};

#endif // RTBCORE_H

}
}
