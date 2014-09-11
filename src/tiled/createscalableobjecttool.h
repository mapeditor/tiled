#ifndef CREATESCALABLEOBJECTTOOL_H
#define CREATESCALABLEOBJECTTOOL_H

#include "createobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateScalableObjectTool : public CreateObjectTool
{
    Q_OBJECT
public:
    CreateScalableObjectTool(QObject* parent);
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);
    void languageChanged();
};

}
}

#endif // CREATESCALABLEOBJECTTOOL_H
