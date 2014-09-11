#ifndef CREATETILEOBJECTTOOL_H
#define CREATETILEOBJECTTOOL_H

#include "createobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateTileObjectTool : public CreateObjectTool
{
    Q_OBJECT
public:
    CreateTileObjectTool(QObject* parent);
    void languageChanged();

protected:
    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers, const bool snapToGrid, const bool snapToFineGrid);
    void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event, const bool snapToGrid, const bool snapToFineGrid);
    void mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event, const bool snapToGrid, const bool snapToFineGrid);
    MapObject* createNewMapObject();
};

}
}

#endif // CREATETILEOBJECTTOOL_H
