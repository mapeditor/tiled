#include "viewinterface.h"

#include "preferences.h"

namespace Tiled {

ViewInterface::ViewInterface(QWidget *parent)
    : QObject{parent}
    , mMapView{std::make_unique<MapView>(parent)}
#ifdef TILEDQUICK_LIB
    , mQuickWidget{std::make_unique<QQuickWidget>(parent)}
#endif
{
}

ViewInterface::~ViewInterface()
{
}

QWidget* ViewInterface::getWidget() const
{
#ifdef TILEDQUICK_LIB
    if (quickEnabled())
        return mQuickWidget.get();
#endif
    return mMapView.get();
}

MapView* ViewInterface::mapView() const
{
    return mMapView.get();
}

#ifdef TILEDQUICK_LIB
QQuickWidget* ViewInterface::quickWidget() const
{
    return mQuickWidget.get();
}
#endif

inline bool ViewInterface::quickEnabled() const
{
    return Preferences::instance()->useNewHardwareRenderer();
}

}