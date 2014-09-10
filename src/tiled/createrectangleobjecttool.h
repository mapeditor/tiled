#ifndef CREATERECTANGLEOBJECTTOOL_H
#define CREATERECTANGLEOBJECTTOOL_H

#include "createobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateRectangleObjectTool : public CreateObjectTool
{
    Q_OBJECT
public:
    CreateRectangleObjectTool(QObject* parent);
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);
    void languageChanged();
};

}
}

#endif // CREATERECTANGLEOBJECTTOOL_H
