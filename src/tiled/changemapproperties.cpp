#include "changemapproperties.h"

#include "map.h"
#include "mapdocument.h"
#include "objectgroup.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeMapProperties::ChangeMapProperties(MapDocument *mapDocument, const QColor &bgColor)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Change Map Properties"))
    , mMapDocument(mapDocument)
    , mUndoColor(mapDocument->map()->backgroundColor())
    , mRedoColor(bgColor)
{
}

void ChangeMapProperties::redo()
{
    mMapDocument->map()->setBackgroundColor(mRedoColor);
    mMapDocument->emitMapChanged();
}

void ChangeMapProperties::undo()
{
    mMapDocument->map()->setBackgroundColor(mUndoColor);
    mMapDocument->emitMapChanged();
}
