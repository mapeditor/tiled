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

    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);
protected:
    MapObject* createNewMapObject(const QPointF &pos);
};

}
}

#endif // CREATETILEOBJECTTOOL_H
