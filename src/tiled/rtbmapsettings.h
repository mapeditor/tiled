/*
 * rtbmapsettings.h
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

#ifndef RTBMAPSETTINGS_H
#define RTBMAPSETTINGS_H

#include "mapdocument.h"
#include "tmxmapreader.h"


namespace Tiled{

namespace Internal{


/**
 * The RTBMap class create an RTB-Map with the needed settings, tilesets and properties
 *
 */
class RTBMapSettings
{

public:
    RTBMapSettings();

    /**
     * Load the needed Tilesets
     */
    void loadTileSets(MapDocument *mapDocument);

    /**
     * Set map settings, e.g background color
     */
    void setMapSettings(Map *map);

    /**
     * Create the needed Layers
     */
    void createLayers(MapDocument *mapDocument);

    /**
     * Add the startet content
     */
    void addStarterContent(MapDocument *mapDocument);

    enum LayerBorder {
        FloorBorder = 23,
        OrbBorder = 39,
        ObjectBorder = 63
    };

    enum LayerIDs {
        FloorID,
        ObjectID,
        OrbObjectID
    };

    enum ToolIDs {
        StampBrush = 2,
        BucketFill = 3,
        ObjectSelection = 6
    };

    enum FloorTiles {
        Floor = 0,
        FloorTrap = 1,
        Barrier = 3,
        HiddenFloor = 4,
        WallBlock = 7,
        SpeedpadRight = 8,
        SpeedpadLeft = 9,
        SpeedpadUp = 10,
        SpeedpadDown = 11,
        Jumppad = 12,
    };

    enum OrbTiles {
        PointOrb = 24,
        CheckpointOrb = 25,
        HealthOrb = 26,
        KeyOrb = 27,
        FakeOrb = 28
    };

    static SharedTileset createTileset()
    {
        TmxMapReader reader;
        return reader.readTileset(QLatin1String(":/rtb_resources/tileset/Floor.tsx"));
    }

    static SharedTileset createActionTileset()
    {
        TmxMapReader reader;
        return reader.readTileset(QLatin1String(":/rtb_resources/icons/Floor_actions.tsx"));
    }

    static SharedTileset tileset()
    {

        if(!mTileset)
        {
            mTileset = createTileset();
            return mTileset;
        }
        else
            return mTileset;
    }

    static SharedTileset actionTileset()
    {

        if(!mActionTileset)
        {
            mActionTileset = createActionTileset();
            return mActionTileset;
        }
        else
            return mActionTileset;
    }

    static QPixmap tileImageSource(int id)
    {
        Tileset *tileset = actionTileset().data();
        return tileset->tileAt(id)->image();
    }

private:
    /**
     * The mapDocument to prepare for the RTB
     */
    MapDocument *mMapDocument;

    static SharedTileset mTileset;
    static SharedTileset mActionTileset;
};

}
}
#endif // RTBMAPSETTINGS_H
