#pragma once

#include <QQuickItem>

#include "tiledquick_global.h"

namespace Tiled {
class MapRenderer;
}

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT MapGridItem : public QQuickItem
{
    Q_OBJECT

public:
    explicit MapGridItem(QQuickItem *parent = nullptr);
    ~MapGridItem() override;

    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;

signals:

private:
    QColor mColor = Qt::black;
    int lineLength = 2;
    int spaceLength = 2;
};

} // namespace TiledQuick
