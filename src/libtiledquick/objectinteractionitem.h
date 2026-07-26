#pragma once

#include <QQuickItem>

#include "mapobject.h"

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT ObjectInteractionItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QList<Tiled::MapObject*> selectedObjects READ selectedObjects WRITE setSelectedObjects NOTIFY selectedObjectsChanged)
    Q_PROPERTY(QList<Tiled::MapObject*> hoveredObjects READ hoveredObjects WRITE setHoveredObjects NOTIFY hoveredObjectsChanged)
    Q_PROPERTY(QList<QPolygonF> selectionOutlines READ selectionOutlines NOTIFY selectionOutlinesChanged)
    Q_PROPERTY(QList<QPolygonF> hoverOutlines READ hoverOutlines NOTIFY hoverOutlinesChanged)

public:
    ObjectInteractionItem(QQuickItem *parent = nullptr);
    ~ObjectInteractionItem() override;

    QList<Tiled::MapObject*> selectedObjects() const;
    void setSelectedObjects(const QList<Tiled::MapObject*> &objects);

    QList<Tiled::MapObject*> hoveredObjects() const;
    void setHoveredObjects(const QList<Tiled::MapObject*> &objects);

    QList<QPolygonF> selectionOutlines() const;
    QList<QPolygonF> hoverOutlines() const;

    Q_INVOKABLE void updateOutlines();

signals:
    void selectedObjectsChanged();
    void hoveredObjectsChanged();
    void selectionOutlinesChanged();
    void hoverOutlinesChanged();

private:
    QList<Tiled::MapObject*> mSelectedObjects;
    QList<Tiled::MapObject*> mHoveredObjects;
};

inline QList<Tiled::MapObject*> ObjectInteractionItem::selectedObjects() const
{
    return mSelectedObjects;
}

inline QList<Tiled::MapObject*> ObjectInteractionItem::hoveredObjects() const
{
    return mHoveredObjects;
}

} // namespace TiledQuick
