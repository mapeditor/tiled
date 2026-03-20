#pragma once

#include <QQuickItem>

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT MapBorderItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit MapBorderItem(QQuickItem *parent = nullptr);
    ~MapBorderItem() override;

    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;

    QColor color() const;
    void setColor(const QColor &color);

signals:
    void colorChanged();

private:
    QColor mColor = Qt::black;
};

} // namespace TiledQuick
