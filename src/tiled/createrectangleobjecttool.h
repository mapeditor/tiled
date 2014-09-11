#ifndef CREATERECTANGLEOBJECTTOOL_H
#define CREATERECTANGLEOBJECTTOOL_H

#include "createscalableobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateRectangleObjectTool : public CreateScalableObjectTool
{
    Q_OBJECT
public:
    CreateRectangleObjectTool(QObject* parent);
    void languageChanged();
protected:
    MapObject* createNewMapObject();
};

}
}

#endif // CREATERECTANGLEOBJECTTOOL_H
