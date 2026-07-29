#pragma once

#include <QQuickItem>

#include "mapobject.h"

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT ObjectInteractionItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QByteArray selectedToolId READ selectedToolId WRITE setSelectedToolId NOTIFY selectedToolIdChanged)
    Q_PROPERTY(QList<Tiled::MapObject*> selectedObjects READ selectedObjects WRITE setSelectedObjects NOTIFY selectedObjectsChanged)
    Q_PROPERTY(QList<Tiled::MapObject*> hoveredObjects READ hoveredObjects WRITE setHoveredObjects NOTIFY hoveredObjectsChanged)
    Q_PROPERTY(QList<QPolygonF> selectionOutlines READ selectionOutlines NOTIFY selectionOutlinesChanged)
    Q_PROPERTY(QList<QPolygonF> hoverOutlines READ hoverOutlines NOTIFY hoverOutlinesChanged)
    Q_PROPERTY(QRectF selectionRect READ selectionRect NOTIFY selectionRectChanged)

public:
    ObjectInteractionItem(QQuickItem *parent = nullptr);
    ~ObjectInteractionItem() override;

    QByteArray selectedToolId() const;
    void setSelectedToolId(const QByteArray &id);

    QList<Tiled::MapObject*> selectedObjects() const;
    void setSelectedObjects(const QList<Tiled::MapObject*> &objects);

    QList<Tiled::MapObject*> hoveredObjects() const;
    void setHoveredObjects(const QList<Tiled::MapObject*> &objects);

    QList<QPolygonF> selectionOutlines() const;
    QList<QPolygonF> hoverOutlines() const;
    QRectF selectionRect() const;

    Q_INVOKABLE QColor selectionRectBorderColor() const;
    Q_INVOKABLE QColor selectionRectFillColor() const;

    Q_INVOKABLE void updateOutlines();
    Q_INVOKABLE void mousePressed(const QPointF &pos);
    Q_INVOKABLE void mouseMoved(const QPointF &pos);
    Q_INVOKABLE void mouseReleased(const QPointF &pos);

signals:
    void selectedToolIdChanged();
    void selectedObjectsChanged();
    void hoveredObjectsChanged();
    void selectionOutlinesChanged();
    void hoverOutlinesChanged();
    void selectionRectChanged();

private:
    QByteArray mSelectedToolId;
    QList<Tiled::MapObject*> mSelectedObjects;
    QList<Tiled::MapObject*> mHoveredObjects;
    bool mMousePressed;
    QRectF mSelectionRect;
};

inline QByteArray ObjectInteractionItem::selectedToolId() const {
    return mSelectedToolId;
}

inline QList<Tiled::MapObject*> ObjectInteractionItem::selectedObjects() const
{
    return mSelectedObjects;
}

inline QList<Tiled::MapObject*> ObjectInteractionItem::hoveredObjects() const
{
    return mHoveredObjects;
}

inline QRectF ObjectInteractionItem::selectionRect() const
{
    return mSelectionRect.normalized();
}

} // namespace TiledQuick
