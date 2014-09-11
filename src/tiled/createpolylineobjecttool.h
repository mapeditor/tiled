#ifndef CREATEPOLYLINEOBJECTTOOL_H
#define CREATEPOLYLINEOBJECTTOOL_H

#include "createmultipointobjecttool.h"

namespace Tiled {

namespace Internal {

class CreatePolylineObjectTool: public CreateMultipointObjectTool
{
    Q_OBJECT
public:
    CreatePolylineObjectTool(QObject* parent);
    void languageChanged();
protected:
    MapObject* createNewMapObject(const QPointF &pos);
    void finishNewMapObject();
};

}
}

#endif // CREATEPOLYLINEOBJECTTOOL_H
