#pragma once

#include <QQuickItem>

#include "mapobject.h"

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT ObjectInteractionItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QList<Tiled::MapObject*> selectedObjects READ selectedObjects WRITE setSelectedObjects NOTIFY selectedObjectsChanged)
    Q_PROPERTY(QList<QPolygonF> selectionOutlines READ selectionOutlines NOTIFY selectedObjectsChanged)
public:
    ObjectInteractionItem(QQuickItem *parent = nullptr);
    ~ObjectInteractionItem() override;

    QList<Tiled::MapObject*> selectedObjects() const;
    void setSelectedObjects(const QList<Tiled::MapObject*> &objects);

    QList<QPolygonF> selectionOutlines() const;

signals:
    void selectedObjectsChanged();

private:
    QList<Tiled::MapObject*> mSelectedObjects;
};

inline QList<Tiled::MapObject*> ObjectInteractionItem::selectedObjects() const
{
    return mSelectedObjects;
}

} // namespace TiledQuick
