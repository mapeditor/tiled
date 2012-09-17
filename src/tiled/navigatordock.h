#ifndef NAVIGATORDOCK_H
#define NAVIGATORDOCK_H


#include <QDockWidget>


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

public:

    NavigatorDock(QWidget* parent=0);
    /***/
    void setMapDocument(MapDocument*);
    /** should be notified whenever zoom/scrollpos has changed */
    void mapViewChanged();
    /** should be notified whenever the content of the map has changed */
    void mapModelChanged();

protected:

    void changeEvent(QEvent *e);

private:

    /** update ui */
    void retranslateUi();
    /** ui object for rnedering */
    NavigatorFrame* mDrawFrame;
    /** link to the current map */
    MapDocument* mMapDocument;


};


} // namespace Internal
} // namespace Tiled

#endif // NAVIGATORDOCK_H
