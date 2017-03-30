#include "minimaprenderer.h"

#include "imagelayer.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "preferences.h"
#include "tilelayer.h"

#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

MiniMapRenderer* MiniMapRenderer::instance()
{
    static MiniMapRenderer* singletonRenderer = new MiniMapRenderer();
    return singletonRenderer;
}

static bool objectLessThan(const MapObject *a, const MapObject *b)
{
    return a->y() < b->y();
}

void MiniMapRenderer::renderMinimapToImage(QImage& image, const MiniMapRenderFlags minimapRenderFlags) const
{
    if (!mMapDocument)
        return;
    
    MapRenderer *renderer = mMapDocument->renderer();

    bool drawObjects = minimapRenderFlags.testFlag(MiniMapRenderFlag::DrawObjects);
    bool drawTiles = minimapRenderFlags.testFlag(MiniMapRenderFlag::DrawTiles);
    bool drawImages = minimapRenderFlags.testFlag(MiniMapRenderFlag::DrawImages);
    bool drawTileGrid = minimapRenderFlags.testFlag(MiniMapRenderFlag::DrawGrid);
    bool visibleLayersOnly = minimapRenderFlags.testFlag(MiniMapRenderFlag::IgnoreInvisibleLayer);

    // Remember the current render flags
    const Tiled::RenderFlags renderFlags = renderer->flags();
    renderer->setFlag(ShowTileObjectOutlines, false);

    QSize mapSize = renderer->mapSize();
    QMargins margins = mMapDocument->map()->computeLayerOffsetMargins();
    mapSize.setWidth(mapSize.width() + margins.left() + margins.right());
    mapSize.setHeight(mapSize.height() + margins.top() + margins.bottom());

    // Determine the largest possible scale
    qreal scale = qMin((qreal) image.width() / mapSize.width(),
                       (qreal) image.height() / mapSize.height());

    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    painter.setTransform(QTransform::fromScale(scale, scale));
    painter.translate(margins.left(), margins.top());
    renderer->setPainterScale(scale);

    LayerIterator iterator(mMapDocument->map());
    while (const Layer *layer = iterator.next()) {
        if (visibleLayersOnly && layer->isHidden())
            continue;

        const auto offset = layer->totalOffset();

        painter.setOpacity(layer->effectiveOpacity());
        painter.translate(offset);

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ObjectGroup *objGroup = dynamic_cast<const ObjectGroup*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer && drawTiles) {
            renderer->drawTileLayer(&painter, tileLayer);
        } else if (objGroup && drawObjects) {
            QList<MapObject*> objects = objGroup->objects();

            if (objGroup->drawOrder() == ObjectGroup::TopDownOrder)
                qStableSort(objects.begin(), objects.end(), objectLessThan);

            foreach (const MapObject *object, objects) {
                if (object->isVisible()) {
                    if (object->rotation() != qreal(0)) {
                        QPointF origin = renderer->pixelToScreenCoords(object->position());
                        painter.save();
                        painter.translate(origin);
                        painter.rotate(object->rotation());
                        painter.translate(-origin);
                    }

                    const QColor color = MapObjectItem::objectColor(object);
                    renderer->drawMapObject(&painter, object, color);

                    if (object->rotation() != qreal(0))
                        painter.restore();
                }
            }
        } else if (imageLayer && drawImages) {
            renderer->drawImageLayer(&painter, imageLayer);
        }

        painter.translate(-offset);
    }

    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        renderer->drawGrid(&painter, QRectF(QPointF(), renderer->mapSize()),
                           prefs->gridColor());
    }

    renderer->setFlags(renderFlags);
}

void MiniMapRenderer::setMapDocument(MapDocument* map)
{
    mMapDocument = map;
}

MiniMapRenderer::MiniMapRenderer() {}
