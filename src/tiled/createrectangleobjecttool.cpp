#include "createrectangleobjecttool.h"
#include "preferences.h"
#include "Utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateRectangleObjectTool::CreateRectangleObjectTool(QObject* parent)
    : CreateObjectTool(CreateObjectTool::CreatePolymorphic, parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-rectangle.png")));
    Utils::setThemeIcon(this, "insert-rectangle");
}

void CreateRectangleObjectTool::mouseMoved(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers)
{
    CreateObjectTool::mouseMoved(pos, modifiers);

    if (!mNewMapObjectItem)
        return;

    const MapRenderer *renderer = mapDocument()->renderer();

    bool snapToGrid = Preferences::instance()->snapToGrid();
    bool snapToFineGrid = Preferences::instance()->snapToFineGrid();
    if (modifiers & Qt::ControlModifier) {
        snapToGrid = !snapToGrid;
        snapToFineGrid = false;
    }

    const QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    // Update the size of the new map object
    const QPointF objectPos = mNewMapObjectItem->mapObject()->position();
    QPointF newSize(qMax(qreal(0), pixelCoords.x() - objectPos.x()),
                    qMax(qreal(0), pixelCoords.y() - objectPos.y()));
    QPointF newTileSize = renderer->pixelToTileCoords(newSize);

    if (snapToFineGrid) {
        int gridFine = Preferences::instance()->gridFine();
        newTileSize = (newTileSize * gridFine).toPoint();
        newTileSize /= gridFine;
    } else if (snapToGrid)
        newTileSize = newTileSize.toPoint();

    // Holding shift creates circle or square
    if (modifiers & Qt::ShiftModifier) {
        qreal max = qMax(newTileSize.x(), newTileSize.y());
        newTileSize.setX(max);
        newTileSize.setY(max);
    }

    newSize = renderer->tileToPixelCoords(newTileSize);

    mNewMapObjectItem->resizeObject(QSizeF(newSize.x(), newSize.y()));
}

void CreateRectangleObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mNewMapObjectItem) {
        if (event->button() == Qt::RightButton)
            cancelNewMapObject();
    }
    CreateObjectTool::mousePressed(event);
    // Check if we are already creating a new map object
}

void CreateRectangleObjectTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    CreateObjectTool::mouseReleased(event);
    // Check if we are already creating a new map object
    if (event->button() == Qt::LeftButton && mNewMapObjectItem) {
        finishNewMapObject();
    }
}

void CreateRectangleObjectTool::languageChanged(){
    setName(tr("Insert Rectangle"));
    setShortcut(QKeySequence(tr("R")));
}
