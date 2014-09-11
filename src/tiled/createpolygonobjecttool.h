#ifndef CREATEPOLYGONOBJECTTOOL_H
#define CREATEPOLYGONOBJECTTOOL_H

#include "createmultipointobjecttool.h"

namespace Tiled {

namespace Internal {

class CreatePolygonObjectTool: public CreateMultipointObjectTool
{
    Q_OBJECT
public:
    CreatePolygonObjectTool(QObject* parent);
    void languageChanged();
protected:
    MapObject* createNewMapObject(const QPointF &pos);
    void finishNewMapObject();
};

}
}

#endif // CREATEPOLYGONOBJECTTOOL_H
