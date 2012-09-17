#include "navigatorframe.h"

#include "NavigatorFrame.h"
#include "maprenderer.h"
#include <QPainter>
#include "objectgroup.h"
#include "mapdocument.h"
#include "map.h"
#include "tilelayer.h"
#include "imagelayer.h"
#include "mapobjectitem.h"
#include "mapview.h"
#include <QResizeEvent>
#include "documentmanager.h"
#include <QDebug>
#include <QScrollBar>
#include "zoomable.h"

using namespace Tiled;
using namespace Tiled::Internal;


NavigatorFrame::NavigatorFrame(QWidget *parent)
    : QFrame(parent)
    , mMapImage(NULL)
    , mMapDocument(NULL)
    , mScrollX(NULL)
    , mScrollY(NULL)
{
    setFrameStyle( QFrame::NoFrame );

}

void NavigatorFrame::setMapDocument(MapDocument *map)
{

    if (mScrollX)
        disconnect(mScrollX, SIGNAL(valueChanged(int)), this, SLOT(scrollbarChanged(int)));
    if (mScrollY)
        disconnect(mScrollY, SIGNAL(valueChanged(int)), this, SLOT(scrollbarChanged(int)));

    MapView* mapView = DocumentManager::instance()->currentMapView();
    if (mapView)
    {
        mScrollX = mapView->horizontalScrollBar();
        mScrollY = mapView->verticalScrollBar();

        connect(mScrollX, SIGNAL(valueChanged(int)), SLOT(scrollbarChanged(int)));
        connect(mScrollY, SIGNAL(valueChanged(int)), SLOT(scrollbarChanged(int)));
    }


    mMapDocument = map;
    redrawMapAndFrame();
}

void NavigatorFrame::redrawFrame()
{
    repaint();
}

void NavigatorFrame::redrawMapAndFrame()
{
    recreateMapImage();
    repaint();
}

void NavigatorFrame::paintEvent(QPaintEvent *pe)
{
    QFrame::paintEvent( pe );
    QPainter p( this );

    QPoint frameCenter = QPoint(size().width(), size().height())*0.5;
    frameCenter -= QPoint(imageContentRect.size().width()+2, imageContentRect.size().height()+2)*0.5;
    p.translate(frameCenter);

    // outline
    p.setBrush(QBrush(QColor(255, 255, 255)));
    p.setPen(Qt::NoPen);
    p.drawRect(imageContentRect.x(),
               imageContentRect.y(),
               imageContentRect.width() + 1,
               imageContentRect.height() + 1);

    if (mMapImage)
        p.drawImage(1, 1, *mMapImage);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(0, 0, 0)));
    p.drawRect(imageContentRect.x(),
               imageContentRect.y(),
               imageContentRect.width() + 1,
               imageContentRect.height() + 1);


    p.setPen(QPen(QBrush(QColor(255, 0, 0)), 2));


    MapView* mapView = DocumentManager::instance()->currentMapView();
    if (mapView)
    {
        // viewRect => the size of the entire scene in pixel
        QRectF mapContentRect = mapView->sceneRect();
        QRectF visibleRect = mapView->mapToScene(mapView->viewport()->geometry()).boundingRect();

        QRectF drawRect = QRectF(
                    visibleRect.x() / mapContentRect.width()* imageContentRect.width(),
                    visibleRect.y() / mapContentRect.height() * imageContentRect.height(),
                    visibleRect.width() / mapContentRect.width() * imageContentRect.width(),
                    visibleRect.height() / mapContentRect.height() * imageContentRect.height());

        p.drawRect(drawRect);
    }
}

void NavigatorFrame::resizeEvent(QResizeEvent *event)
{
    recreateMapImage();
}



void NavigatorFrame::recreateMapImage()
{
    QSize newImageSize = size();
    if (size().width() < 50 ||
        size().height() < 50)
        newImageSize = QSize(50, 50);

    if (mMapImage == NULL)
        mMapImage = new QImage(newImageSize, QImage::Format_ARGB32);
    else if (mMapImage->size().width() < newImageSize.width() ||
             mMapImage->size().height() < newImageSize.height())
        resizeImage(newImageSize);

    if (mMapDocument)
        renderMapToImage();
}

void NavigatorFrame::renderMapToImage()
{

    if (size().width() < 2 ||
        size().height() < 2)
        return;

    MapRenderer *renderer = mMapDocument->renderer();

    bool visibleLayersOnly = true;


    // Remember the current render flags
    const Tiled::RenderFlags renderFlags = renderer->flags();



    renderer->setFlag(ShowTileObjectOutlines, false);


    mMapImage->fill(Qt::transparent);
    QPainter painter(mMapImage);


    QTransform trans;
    QSizeF mapSize = renderer->mapSize();
    QSizeF frameSize = size() - QSizeF(2, 2);
    float xScale = frameSize.width() / mapSize.width();
    float yScale = frameSize.height() / mapSize.height();
    int tx = mapSize.width() * xScale;
    int ty = mapSize.height() * xScale;
    if (tx <= frameSize.width() &&
        ty <= frameSize.height())
    {
        imageContentRect.setSize(QSize(tx,ty));
        trans.scale(xScale, xScale);
    }
    else
    {
        imageContentRect.setSize(QSize(mapSize.width() * yScale, mapSize.height() * yScale));
        trans.scale(yScale, yScale);
    }
    painter.setTransform(trans);



    foreach (const Layer *layer, mMapDocument->map()->layers()) {
        if (visibleLayersOnly && !layer->isVisible())
            continue;

        painter.setOpacity(layer->opacity());

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ObjectGroup *objGroup = dynamic_cast<const ObjectGroup*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer) {
            renderer->drawTileLayer(&painter, tileLayer);
        }
        /*
        else if (objGroup) {
            foreach (const MapObject *object, objGroup->objects()) {
                const QColor color = MapObjectItem::objectColor(object);
                renderer->drawMapObject(&painter, object, color);
            }
        } */
        else if (imageLayer) {
            renderer->drawImageLayer(&painter, imageLayer);
        }
    }

    /*
    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        renderer->drawGrid(&painter, QRectF(QPointF(), renderer->mapSize()),
                          prefs->gridColor());
    }*/

    // Restore the previous render flags
    renderer->setFlags(renderFlags);

}

void NavigatorFrame::resizeImage(const QSize &newSize)
{
    if (mMapImage->size() == newSize)
        return;

    delete mMapImage;
    mMapImage = new QImage(newSize, QImage::Format_ARGB32);
}

void NavigatorFrame::scrollbarChanged(int xx)
{
    update();
}

void NavigatorFrame::wheelEvent(QWheelEvent *event)
{
    MapView* mapView = DocumentManager::instance()->currentMapView();


    if (mapView &&
        event->orientation() == Qt::Vertical)
    {



        QPoint frameCenter = QPoint(size().width(), size().height())*0.5;
        frameCenter -= QPoint(imageContentRect.size().width()+2, imageContentRect.size().height()+2)*0.5;
        QPoint cursorPos = event->pos() - frameCenter;

        // viewRect => the size of the entire scene in pixel
        QRectF mapContentRect = mapView->sceneRect();
        QRectF visibleRect = mapView->mapToScene(mapView->viewport()->geometry()).boundingRect();

        QRectF drawRect = QRectF(
                    visibleRect.x() / mapContentRect.width()* imageContentRect.width(),
                    visibleRect.y() / mapContentRect.height() * imageContentRect.height(),
                    visibleRect.width() / mapContentRect.width() * imageContentRect.width(),
                    visibleRect.height() / mapContentRect.height() * imageContentRect.height());

        if (cursorPos.x() >= 0)
            qWarning() << "wheel baby";

        mapView->setTransformationAnchor(QGraphicsView::NoAnchor);
        mapView->zoomable()->handleWheelDelta(event->delta());

        /*
        QPointF center = drawRect.topLeft();
        center.setX(center.x() + drawRect.width()*0.5);
        center.setY(center.y() + drawRect.height()*0.5);
        */

        QPointF center = mapView->sceneRect().center();

        mapView->centerOn(center);

        mapView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

        update();
        /*
        // No automatic anchoring since we'll do it manually
        setTransformationAnchor(QGraphicsView::NoAnchor);

        mZoomable->handleWheelDelta(event->delta());

        // Place the last known mouse scene pos below the mouse again
        QWidget *view = viewport();
        QPointF viewCenterScenePos = mapToScene(view->rect().center());
        QPointF mouseScenePos = mapToScene(view->mapFromGlobal(mLastMousePos));
        QPointF diff = viewCenterScenePos - mouseScenePos;
        centerOn(mLastMouseScenePos + diff);

        // Restore the centering anchor
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        */

        return;
    }

    QFrame::wheelEvent(event);
}

void NavigatorFrame::mousePressEvent(QMouseEvent *event)
{
}

void NavigatorFrame::mouseReleaseEvent(QMouseEvent *event)
{
}

void NavigatorFrame::mouseMoveEvent(QMouseEvent *event)
{
}

