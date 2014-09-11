#ifndef CREATEMULTIPOINTBJECTTOOL_H
#define CREATEMULTIPOINTBJECTTOOL_H

#include "createobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateMultipointObjectTool : public CreateObjectTool
{
    Q_OBJECT
public:
    CreateMultipointObjectTool(QObject* parent);
    void startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup);
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void languageChanged() = 0;
};

}
}

#endif // CREATEMULTIPOINTBJECTTOOL_H
