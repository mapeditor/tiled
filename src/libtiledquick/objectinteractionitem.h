#pragma once

#include <QQuickItem>

#include "mapobject.h"
#include "objectselectiontool.h"

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT ObjectInteractionItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QString selectedToolId READ selectedToolId WRITE setSelectedToolId NOTIFY selectedToolIdChanged)
    Q_PROPERTY(QList<Tiled::MapObject*> selectedObjects READ selectedObjects WRITE setSelectedObjects NOTIFY selectedObjectsChanged)
    Q_PROPERTY(QList<Tiled::MapObject*> hoveredObjects READ hoveredObjects WRITE setHoveredObjects NOTIFY hoveredObjectsChanged)
    Q_PROPERTY(QList<QPointF> selectedPolygonEditPoints READ selectedPolygonEditPoints WRITE setSelectedPolygonEditPoints NOTIFY selectedPolygonEditPointsChanged)
    Q_PROPERTY(QList<QPointF> highlightedPolygonEditPoints READ highlightedPolygonEditPoints WRITE setHighlightedPolygonEditPoints NOTIFY highlightedPolygonEditPointsChanged)
    Q_PROPERTY(QList<Tiled::ObjectHandleData> objectHandles READ objectHandles WRITE setObjectHandles NOTIFY objectHandlesChanged)

    Q_PROPERTY(QList<QPolygonF> selectionOutlines READ selectionOutlines NOTIFY selectionOutlinesChanged)
    Q_PROPERTY(QList<QPolygonF> hoverOutlines READ hoverOutlines NOTIFY hoverOutlinesChanged)
    Q_PROPERTY(QRectF selectionRect READ selectionRect NOTIFY selectionRectChanged)
    Q_PROPERTY(QList<QPolygonF> objectHandlePolygons READ objectHandlePolygons NOTIFY objectHandlesChanged)
    Q_PROPERTY(QPolygonF hoveredHandlePolygon READ hoveredHandlePolygon NOTIFY objectHandlesChanged)
    Q_PROPERTY(QList<QPointF> polygonEditPoints READ polygonEditPoints NOTIFY polygonEditPointsChanged)

public:
    ObjectInteractionItem(QQuickItem *parent = nullptr);
    ~ObjectInteractionItem() override;

    qreal zoom() const;
    void setZoom(const qreal zoom);

    QString selectedToolId() const;
    void setSelectedToolId(const QString &id);

    QList<Tiled::MapObject*> selectedObjects() const;
    void setSelectedObjects(const QList<Tiled::MapObject*> &objects);

    QList<Tiled::MapObject*> hoveredObjects() const;
    void setHoveredObjects(const QList<Tiled::MapObject*> &objects);

    QList<QPointF> selectedPolygonEditPoints() const;
    void setSelectedPolygonEditPoints(const QList<QPointF> &points);

    QList<QPointF> highlightedPolygonEditPoints() const;
    void setHighlightedPolygonEditPoints(const QList<QPointF> &points);

    QList<Tiled::ObjectHandleData> objectHandles() const;
    void setObjectHandles(const QList<Tiled::ObjectHandleData> &handles);

    QList<QPolygonF> selectionOutlines() const;
    QList<QPolygonF> hoverOutlines() const;
    QRectF selectionRect() const;
    QList<QPolygonF> objectHandlePolygons() const;
    QPolygonF hoveredHandlePolygon() const;
    QList<QPointF> polygonEditPoints() const;

    Q_INVOKABLE void updateOutlines();
    Q_INVOKABLE void mousePressed(const QPointF &pos);
    Q_INVOKABLE void mouseMoved(const QPointF &pos);
    Q_INVOKABLE void mouseReleased(const QPointF &pos);

    Q_INVOKABLE QColor selectionRectBorderColor() const;
    Q_INVOKABLE QColor selectionRectFillColor() const;

signals:
    void zoomChanged();
    void selectedToolIdChanged();
    void selectedObjectsChanged();
    void hoveredObjectsChanged();
    void selectedPolygonEditPointsChanged();
    void highlightedPolygonEditPointsChanged();
    void objectHandlesChanged();
    void selectionOutlinesChanged();
    void hoverOutlinesChanged();
    void selectionRectChanged();
    void polygonEditPointsChanged();

private:
    qreal mZoom = 0;
    QString mSelectedToolId;
    QList<Tiled::MapObject*> mSelectedObjects;
    QList<Tiled::MapObject*> mHoveredObjects;
    QList<QPointF> mSelectedPolygonEditPoints;
    QList<QPointF> mHighlightedPolygonEditPoints;
    QList<Tiled::ObjectHandleData> mObjectHandles;
    bool mDrawSelectionRect;
    QRectF mSelectionRect;
};

inline qreal ObjectInteractionItem::zoom() const {
    return mZoom;
}

inline QString ObjectInteractionItem::selectedToolId() const {
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

inline QList<QPointF> ObjectInteractionItem::selectedPolygonEditPoints() const
{
    return mSelectedPolygonEditPoints;
}

inline QList<QPointF> ObjectInteractionItem::highlightedPolygonEditPoints() const
{
    return mHighlightedPolygonEditPoints;
}

inline QList<Tiled::ObjectHandleData> ObjectInteractionItem::objectHandles() const
{
    return mObjectHandles;
}

inline QRectF ObjectInteractionItem::selectionRect() const
{
    return mSelectionRect.normalized();
}

} // namespace TiledQuick
