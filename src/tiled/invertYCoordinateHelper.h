#ifndef INVERTYCOORDINATEHELPER_H
#define INVERTYCOORDINATEHELPER_H

#include "preferences.h"                // To get the status of the invertYCoordinate checkBox
#include "mapdocument.h"                // To access the map
#include "mapdocumentactionhandler.h"


class InvertYCoordinateHelper
{
public:
    float getGridY( float y ) const;
    float getPixelY( float y ) const;
    InvertYCoordinateHelper( void );
    ~InvertYCoordinateHelper( void );
};


#endif // INVERTYCOORDINATEHELPER_H
