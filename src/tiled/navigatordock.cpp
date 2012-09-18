#include "navigatordock.h"

#include <QEvent>
#include <QUndoView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsView>
#include <QTimer>

#include "navigatorframe.h"
#include "mapdocument.h"

using namespace Tiled;
using namespace Tiled::Internal;

NavigatorDock::NavigatorDock(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QLatin1String("navigatorDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    mDrawFrame = new NavigatorFrame(this);
    layout->addWidget(mDrawFrame);

    connect(&mUpdateSuspendTimer, SIGNAL(timeout()),
            SLOT(redrawTimeout()));

    setWidget(widget);
    retranslateUi();
}
 
void NavigatorDock::setMapDocument(MapDocument *map)
{
    mMapDocument = map;
    mDrawFrame->setMapDocument(map);
}

void NavigatorDock::mapViewChanged()
{
    mDrawFrame->redrawFrame();
}

void NavigatorDock::mapModelChanged(bool buffered)
{
    if (buffered)
        mUpdateSuspendTimer.start(100);
    else
        mDrawFrame->redrawMapAndFrame();
}

void NavigatorDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void NavigatorDock::retranslateUi()
{
    setWindowTitle(tr("Navigator"));
}

void NavigatorDock::redrawTimeout()
{
    mDrawFrame->redrawMapAndFrame();
    mUpdateSuspendTimer.stop();
}
