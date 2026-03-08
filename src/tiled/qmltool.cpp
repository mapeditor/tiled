#include "qmltool.h"
#include <QGraphicsSceneMouseEvent>
#include "mapscene.h"
#include <QIcon>
#include <QKeySequence>
#include "id.h"

namespace Tiled {

QmlTool::QmlTool(QObject *parent)
    : AbstractTool(Id("qmltool"),
                   QStringLiteral("QML Tool"),
                   QIcon(),
                   QKeySequence(),
                   parent)
{
}

QString QmlTool::iconSource() const
{
    return mIconSource;
}

void QmlTool::setIconSource(const QString &source)
{
    mIconSource = source;

    // load icon from theme (same way Tiled tools do)
    setIcon(QIcon::fromTheme(source));
}

void QmlTool::mouseEntered()
{
    emit qmlMouseEntered();
}

void QmlTool::mouseLeft()
{
    emit qmlMouseLeft();
}

void QmlTool::mouseMoved(const QPointF &pos,
                         Qt::KeyboardModifiers)
{
    emit qmlMouseMoved(pos.x(), pos.y());
}

void QmlTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    emit qmlMousePressed(event->scenePos().x(),
                         event->scenePos().y());
}

void QmlTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    emit qmlMouseReleased(event->scenePos().x(),
                          event->scenePos().y());
}

void QmlTool::languageChanged()
{
}

}
