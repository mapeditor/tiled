#pragma once

#include "abstracttool.h"

namespace Tiled {

class QmlTool : public AbstractTool
{
    Q_OBJECT

    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource)

public:
    explicit QmlTool(QObject *parent = nullptr);

    QString iconSource() const;
    void setIconSource(const QString &source);

    void mouseEntered() override;
    void mouseLeft() override;

    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

signals:

    void qmlMouseEntered();
    void qmlMouseLeft();

    void qmlMouseMoved(int x, int y);
    void qmlMousePressed(int x, int y);
    void qmlMouseReleased(int x, int y);

private:
    QString mIconSource;
};

}
