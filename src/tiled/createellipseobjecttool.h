#ifndef CREATEELLIPSEOBJECTTOOL_H
#define CREATEELLIPSEOBJECTTOOL_H

#include "createscalableobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateEllipseObjectTool : public CreateScalableObjectTool
{
    Q_OBJECT
public:
    CreateEllipseObjectTool(QObject* parent);
    void languageChanged();
protected:
    MapObject* createNewMapObject(const QPointF &pos);
};

}
}

#endif // CREATEELLIPSEOBJECTTOOL_H
