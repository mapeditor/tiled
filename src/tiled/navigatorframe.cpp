#include "navigatorframe.h"
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
#include <QApplication>
#include <QCursor>
#include "preferences.h"

using namespace Tiled;
using namespace Tiled::Internal;


NavigatorFrame::NavigatorFrame(QWidget *parent)
    : QFrame(parent)
    , mMapImage(NULL)
    , mMapDocument(NULL)
    , mScrollX(NULL)
    , mScrollY(NULL)
    , mDragging(false)
    , mMouseMoveCursorState(false)
{
    setFrameStyle( QFrame::NoFrame );
    mRenderFlags = NavigatorRenderFlags(DrawTiles | DrawObjects | DrawImages | IgnoreInvisbleLayer);

    // for cursor changes
    this->setMouseTracking(true);
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
    update();
}

void NavigatorFrame::redrawMapAndFrame()
{
    recreateMapImage();
    update();
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


    MapView* mapView = DocumentManager::instance()->currentMapView();
    if (mapView)
    {
        QRectF drawRect = getViewportRect();
        p.setPen(QColor(0,0,0));
        p.translate(0, 1.5);
        p.drawRect(drawRect);

        p.translate(0, -1.5);
        p.setPen(QPen(QBrush(QColor(255, 0, 0)), 2));
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
        size().height() < 2 ||
        !isVisible())
        return;

    MapRenderer *renderer = mMapDocument->renderer();


    bool drawObjects = mRenderFlags.testFlag(DrawObjects);
    bool drawTiles = mRenderFlags.testFlag(DrawTiles);
    bool drawImages = mRenderFlags.testFlag(DrawImages);
    bool drawTileGrid = mRenderFlags.testFlag(DrawGrid);
    bool visibleLayersOnly = mRenderFlags.testFlag(IgnoreInvisbleLayer);


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

        if (tileLayer && drawTiles) {
            renderer->drawTileLayer(&painter, tileLayer);
        }        
        else if (objGroup && drawObjects) {
            foreach (const MapObject *object, objGroup->objects()) {
                const QColor color = MapObjectItem::objectColor(object);
                renderer->drawMapObject(&painter, object, color);
            }
        }
        else if (imageLayer && drawImages) {
            renderer->drawImageLayer(&painter, imageLayer);
        }
    }


    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        renderer->drawGrid(&painter, QRectF(QPointF(), renderer->mapSize()),
                          prefs->gridColor());
    }

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

void NavigatorFrame::centerViewOnLocalPixel(QPointF centerPos, int delta)
{
    MapView* mapView = DocumentManager::instance()->currentMapView();
    if (!mapView)
        return;

    // viewRect => the size of the entire scene in pixel
    QRectF mapContentRect = mapView->sceneRect();
    mapView->setTransformationAnchor(QGraphicsView::NoAnchor);
    if (delta != 0)
        mapView->zoomable()->handleWheelDelta(delta);
    centerPos.setX(centerPos.x() / imageContentRect.width() * mapContentRect.width());
    centerPos.setY(centerPos.y() / imageContentRect.height() * mapContentRect.height());
    mapView->centerOn(centerPos);
    mapView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

void NavigatorFrame::scrollbarChanged(int xx)
{
    update();
}

void NavigatorFrame::wheelEvent(QWheelEvent *event)
{    
    if (event->orientation() == Qt::Vertical)
    {
        // cursor position on map
        QPoint frameCenter = QPoint(size().width(), size().height())*0.5;
        frameCenter -= QPoint(imageContentRect.size().width()+2, imageContentRect.size().height()+2)*0.5;
        QPointF cursorPos = event->pos() - frameCenter;

        centerViewOnLocalPixel(cursorPos, event->delta());
        update();
        return;
    }

    QFrame::wheelEvent(event);
}

void NavigatorFrame::mousePressEvent(QMouseEvent *event)
{       

    if (event->button() == Qt::LeftButton)
    {
        // cursor position on map
        QPoint frameCenter = QPoint(size().width(), size().height())*0.5;
        frameCenter -= QPoint(imageContentRect.size().width()+2, imageContentRect.size().height()+2)*0.5;
        QPointF cursorPos = event->pos() - frameCenter;

        if (cursorPos.x() >=0 &&
            cursorPos.y() >=0 &&
            cursorPos.x() < imageContentRect.width() &&
            cursorPos.y() < imageContentRect.height())
        {

            QRectF viewPort = getViewportRect();
            if (viewPort.contains(cursorPos))
            {
                mDragOffset = viewPort.center() - cursorPos;
                cursorPos += mDragOffset;
                centerViewOnLocalPixel(cursorPos);
                mDragging = true;

                QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
            }
            else
            {
                centerViewOnLocalPixel(cursorPos);
            }
        }
    }
}

void NavigatorFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (mDragging)
        {
            mDragging = false;
            QApplication::restoreOverrideCursor();
        }
    }
}

void NavigatorFrame::mouseMoveEvent(QMouseEvent *event)
{    
    // cursor position on map
    QPoint frameCenter = QPoint(size().width(), size().height())*0.5;
    frameCenter -= QPoint(imageContentRect.size().width()+2, imageContentRect.size().height()+2)*0.5;
    QPointF cursorPos = event->pos() - frameCenter;

    if (mDragging)
    {
        cursorPos += mDragOffset;
        centerViewOnLocalPixel(cursorPos);
    }
    else
    {
        if (cursorPos.x() >=0 &&
            cursorPos.y() >=0 &&
            cursorPos.x() < imageContentRect.width() &&
            cursorPos.y() < imageContentRect.height())
        {

            QRectF viewPort = getViewportRect();
            if (viewPort.contains(cursorPos))
            {
                if (!mMouseMoveCursorState)
                {
                    QApplication::setOverrideCursor(QCursor(Qt::OpenHandCursor));
                    mMouseMoveCursorState = true;
                }
            }
            else
            {
                if (mMouseMoveCursorState)
                {
                    QApplication::restoreOverrideCursor();
                    mMouseMoveCursorState = false;
                }
            }
        }
    }
}

void NavigatorFrame::leaveEvent(QEvent *e)
{
    QFrame::leaveEvent(e);
    if (mMouseMoveCursorState)
    {
        QApplication::restoreOverrideCursor();
        mMouseMoveCursorState = false;
    }
}

QRectF NavigatorFrame::getViewportRect()
{
    MapView* mapView = DocumentManager::instance()->currentMapView();
    if (!mapView)
        return QRectF(0,0, 1, 1);

    // viewRect => the size of the entire scene in pixel
    QRectF mapContentRect = mapView->sceneRect();
    QRectF visibleRect = mapView->mapToScene(mapView->viewport()->geometry()).boundingRect();
    return QRectF(
                visibleRect.x() / mapContentRect.width()* imageContentRect.width(),
                visibleRect.y() / mapContentRect.height() * imageContentRect.height(),
                visibleRect.width() / mapContentRect.width() * imageContentRect.width(),
                visibleRect.height() / mapContentRect.height() * imageContentRect.height());
}

