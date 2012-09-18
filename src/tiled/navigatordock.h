#ifndef NAVIGATORDOCK_H
#define NAVIGATORDOCK_H


#include <QDockWidget>
#include <QTimer>

namespace Tiled {
namespace Internal {

class NavigatorFrame;
class MapDocument;
class MapView;

/**
 *
 */
class NavigatorDock: public QDockWidget
{
    Q_OBJECT

public:

    NavigatorDock(QWidget* parent=0);
    void setMapDocument(MapDocument*);
    /** should be notified whenever zoom/scrollpos has changed */
    void mapViewChanged();
    /**
     * Should be notified whenever the content of the map has changed.
     *
     * @param buffered true: function uses a timer to prevent high frequent redrawing
     */
    void mapModelChanged(bool buffered);

protected:

    void changeEvent(QEvent *e);

private slots:

    void redrawTimeout();

private:

    /** update ui */
    void retranslateUi();    

    /** ui object for rnedering */
    NavigatorFrame* mDrawFrame;
    /** link to the current map */
    MapDocument* mMapDocument;
    /** to prevent redraw peeks */
    QTimer mUpdateSuspendTimer;

};


} // namespace Internal
} // namespace Tiled

#endif // NAVIGATORDOCK_H 
