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

    NavigatorFrame(QWidget*);
    void setMapDocument(MapDocument*);
    /** just updates the content. Map is unchanged */
    void redrawFrame();
    /** redraws the minimap image and the scroll rectanlge  */
    void redrawMapAndFrame();

protected:

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);

    void wheelEvent(QWheelEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:

    MapDocument *mMapDocument;
    QImage* mMapImage;
    QRect imageContentRect;
    QScrollBar* mScrollX;
    QScrollBar* mScrollY;

    void recreateMapImage();
    void renderMapToImage();
    void resizeImage(const QSize &newSize);


public slots:

    void scrollbarChanged(int xx);


};


} // namespace Internal
} // namespace Tiled

#endif // NAVIGATORFRAME_H
