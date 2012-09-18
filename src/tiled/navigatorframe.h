#ifndef NAVIGATORFRAME_H
#define NAVIGATORFRAME_H


#include <QFrame>
#include <QImage>
#include <QScrollBar>

namespace Tiled {
namespace Internal {

class MapDocument;


class NavigatorFrame: public QFrame
{
    Q_OBJECT

public:

    enum NavigatorRenderFlag
    {
        DrawObjects             = 0x0001,
        DrawTiles               = 0x0002,
        DrawImages              = 0x0004,
        IgnoreInvisbleLayer     = 0x0008,
        DrawGrid                = 0x0010
    };
    Q_DECLARE_FLAGS(NavigatorRenderFlags, NavigatorRenderFlag)

    NavigatorFrame(QWidget*);
    void setMapDocument(MapDocument*);
    /** just updates the content. Map is unchanged */
    void redrawFrame();
    /** redraws the minimap image and the scroll rectanlge  */
    void redrawMapAndFrame();

    NavigatorRenderFlags renderFlags() const { return mRenderFlags; }
    void setRenderFlags(NavigatorRenderFlags flags) { mRenderFlags = flags; }

protected:

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);

private:

    MapDocument *mMapDocument;
    QImage* mMapImage;
    QRect imageContentRect;
    QScrollBar* mScrollX;
    QScrollBar* mScrollY;
    bool mDragging;
    QPointF mDragOffset;
    bool mMouseMoveCursorState;
    NavigatorRenderFlags mRenderFlags;

    QRectF getViewportRect();
    void recreateMapImage();
    void renderMapToImage();
    void resizeImage(const QSize &newSize);
    void centerViewOnLocalPixel(QPointF centerPos, int delta=0);


public slots:

    void scrollbarChanged(int xx);


};


} // namespace Internal
} // namespace Tiled

#endif // NAVIGATORFRAME_H
