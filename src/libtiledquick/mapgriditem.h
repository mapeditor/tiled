#pragma once

#include <QQuickItem>

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT MapGridItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QPointF gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit MapGridItem(QQuickItem *parent = nullptr);
    ~MapGridItem() override;

    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;

    QPointF gridSize() const;
    void setGridSize(const QPointF &gridSize);

    qreal scale() const;
    void setScale(const qreal &scale);

    QColor color() const;
    void setColor(const QColor &color);

signals:
    void gridSizeChanged();
    void scaleChanged();
    void colorChanged();

private:
    QPointF mGridSize = {0,0};
    qreal mScale = 0;
    QColor mColor = Qt::black;
};

} // namespace TiledQuick
