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

    Q_PROPERTY(QPointF gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)

public:
    explicit MapGridItem(QQuickItem *parent = nullptr);
    ~MapGridItem() override;

    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;

    QPointF gridSize() const;
    void setGridSize(const QPointF &gridSize);

    qreal scale() const;
    void setScale(const qreal &scale);

signals:
    void gridSizeChanged();
    void scaleChanged();

private:
    QPointF mGridSize = {0,0};
    qreal mScale = 0;

private:
    QColor mColor = Qt::red;
    int mSegmentLength = 2;
};

} // namespace TiledQuick
