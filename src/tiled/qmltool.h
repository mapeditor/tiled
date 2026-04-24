#pragma once

#include "abstracttool.h"
#include <QPainter>

namespace Tiled {

class QmlTool : public AbstractTool
{
    Q_OBJECT

    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource)
    Q_PROPERTY(int cursorShape READ cursorShape WRITE setCursorShape)
    Q_PROPERTY(QObject* options READ options WRITE setOptions)

public:
    explicit QmlTool(QObject *parent = nullptr);

    QString iconSource() const;
    void setIconSource(const QString &source);

    int cursorShape() const;
    void setCursorShape(int shape);

    QObject* options() const;
    void setOptions(QObject *opt);

    void mouseEntered() override;
    void mouseLeft() override;

    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

    void drawOverlay(QPainter *painter, const QRectF &exposed) ;

signals:

    void qmlMouseEntered();
    void qmlMouseLeft();

    void qmlMouseMoved(int x, int y);
    void qmlMousePressed(int x, int y);
    void qmlMouseReleased(int x, int y);

    void qmlDrawOverlay(QPainter *painter);

private:

    QString mIconSource;
    int mCursorShape = Qt::ArrowCursor;
    QObject *mOptions = nullptr;
};

}

