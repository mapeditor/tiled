#include "viewinterface.h"

#include "preferences.h"

namespace Tiled {

ViewInterface::ViewInterface(QWidget *parent)
    : QObject{parent}
    , mMapView{std::make_unique<MapView>(parent)}
    , mQuickWidget{std::make_unique<QQuickWidget>(parent)}
{
}

ViewInterface::~ViewInterface()
{
}

QWidget* ViewInterface::getWidget() const
{
    if (quickEnabled())
        return mQuickWidget.get();
    return mMapView.get();
}

MapView* ViewInterface::mapView() const
{
    return mMapView.get();
}

QQuickWidget* ViewInterface::quickWidget() const
{
    return mQuickWidget.get();
}

inline bool ViewInterface::quickEnabled() const
{
    return Preferences::instance()->useNewHardwareRenderer();
}

}