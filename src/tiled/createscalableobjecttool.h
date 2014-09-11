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
        void languageChanged();
protected:
    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers, const bool snapToGrid, const bool snapToFineGrid);
    void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event, const bool snapToGrid, const bool snapToFineGrid);
    void mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event, const bool snapToGrid, const bool snapToFineGrid);

};

}
}

#endif // CREATESCALABLEOBJECTTOOL_H
