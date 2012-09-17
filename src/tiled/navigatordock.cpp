#include "navigatordock.h"

#include <QEvent>
#include <QUndoView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsView>

#include "navigatorframe.h"
#include "mapdocument.h"

using namespace Tiled;
using namespace Tiled::Internal;

NavigatorDock::NavigatorDock(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QLatin1String("navigatorDock"));

//    QIcon cleanIcon(QLatin1String(":images/16x16/drive-harddisk.png"));
//    mUndoView->setCleanIcon(cleanIcon);
 //   mUndoView->setUniformItemSizes(true);
  //  mUndoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);


    mDrawFrame = new NavigatorFrame(this);

    layout->addWidget(mDrawFrame);

    //QPushButton* bb = new QPushButton();
    //bb->setText(QLatin1String("yay"));
    //layout->addWidget(bb);

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

void NavigatorDock::mapModelChanged()
{
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
    setWindowTitle(tr("NavigatorDock"));
    //mUndoView->setEmptyLabel(tr("<empty>"));
}
