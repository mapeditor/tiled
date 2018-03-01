/*
 * CoordinateInversionHelper.h
 * The CoordinateInversionHelper is meant to solve issue / feature request #249 "Invert Y axis option".
 * This class will be applied throughout the code whenever the coordinate of an Object or Tile should be viewed (such as in the status bar)
 * or modified (such as in the properties view). Becuase no underlying system should be changed, this class must convert the users input from
 * the inverted format to the standard to save coordinates, as well as from stansard to inverted when displaying the coordinates to the user from
 * the standard coordinate system. This class also requires access to the preferences because that of course is where the user will select whether or not
 * they would like this feature activated. The main purpose of this class is to abstract the Preferences class from all the places in code that this feature
 * will affect.
 * Author: Anthony Divitto
 *  - Under leadership from Thorbjorne the Author of Tiled :)
 *
 *
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#ifndef COORDINATEINVERSIONHELPER_H
#define COORDINATEINVERSIONHELPER_H

#include "preferences.h"

class CoordinateInversionHelper {
public:
    CoordinateInversionHelper( void );
    ~CoordinateInversionHelper( void );

    //## Interface

    //## SIGNALS

    //## SLOTS
private:
    //## Attributes
    bool mInvertYCoordinate;
};
#endif // COORDINATEINVERSIONHELPER_H
