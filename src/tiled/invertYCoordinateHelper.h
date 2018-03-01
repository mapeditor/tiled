#ifndef INVERTYCOORDINATEHELPER_H
#define INVERTYCOORDINATEHELPER_H

#include "preferences.h"                // To get the status of the invertYCoordinate checkBox
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"   // To access the map


struct InvertYCoordinateHelper{
    float getGridY( float y ) const {
        auto map = Tiled::Internal::MapDocumentActionHandler::instance()->mapDocument()->map();
        if (Tiled::Internal::Preferences::instance()->invertYCoordinates()) { // return Y in Bottom-Up format
            return map->tileHeight() * (map->height() + 1) - y;
        }
        return y - map->tileHeight();
    }
    float getPixelY( float y ) const {
        if (Tiled::Internal::Preferences::instance()->invertYCoordinates()) // return Y in Bottom-Up format
            return Tiled::Internal::MapDocumentActionHandler::instance()->mapDocument()->map()->height() - y - 1;
        return y;
    }
    InvertYCoordinateHelper( void ) { }
    ~InvertYCoordinateHelper( void ) { }
};

#endif // INVERTYCOORDINATEHELPER_H
