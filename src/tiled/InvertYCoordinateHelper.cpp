#include "invertYCoordinateHelper.h"

InvertYCoordinateHelper::InvertYCoordinateHelper(void)
{
}

InvertYCoordinateHelper::~InvertYCoordinateHelper(void)
{
}

float InvertYCoordinateHelper::getGridY(float y) const
{
    auto map = Tiled::Internal::MapDocumentActionHandler::instance()->mapDocument()->map();
    if (Tiled::Internal::Preferences::instance()->invertYCoordinates())
        return map->tileHeight() * (map->height() + 1) - y;
    return y - map->tileHeight();
}

float InvertYCoordinateHelper::getPixelY(float y) const
{
    if (Tiled::Internal::Preferences::instance()->invertYCoordinates())
        return Tiled::Internal::MapDocumentActionHandler::instance()->mapDocument()->map()->height() - y - 1;
    return y;
}
