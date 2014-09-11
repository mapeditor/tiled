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
    void languageChanged() = 0;

protected:
    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers, const bool snapToGrid, const bool snapToFineGrid);
    void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event, const bool snapToGrid, const bool snapToFineGrid);
};

}
}

#endif // CREATEMULTIPOINTBJECTTOOL_H
