#pragma once

#ifdef TILEDQUICK_LIB
#include <QQuickWidget>
#endif

#include "mapview.h"

namespace Tiled {

class ViewInterface : public QObject
{
    Q_OBJECT

public:
    ViewInterface(QWidget *parent = nullptr);
    ~ViewInterface() override;

    QWidget* getWidget() const;

    MapView* mapView() const;

#ifdef TILEDQUICK_LIB
    QQuickWidget* quickWidget() const;
#endif

signals:

private:
    inline bool quickEnabled() const;

    std::unique_ptr<MapView> mMapView;
#ifdef TILEDQUICK_LIB
    std::unique_ptr<QQuickWidget> mQuickWidget;
#endif
};

} // namespace Tiled