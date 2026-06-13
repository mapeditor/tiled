#pragma once

#include <QQuickWidget>

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
    QQuickWidget* quickWidget() const;

signals:

private:
    inline bool quickEnabled() const;

    std::unique_ptr<MapView> mMapView;
    std::unique_ptr<QQuickWidget> mQuickWidget;
};

} // namespace Tiled