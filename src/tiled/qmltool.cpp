#include "qmltool.h"

#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QKeySequence>
#include <QCursor>

#include "mapscene.h"
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

    if (source.startsWith(QStringLiteral(":/")) || source.startsWith(QStringLiteral("file")) )
        setIcon(QIcon(source));
    else
        setIcon(QIcon::fromTheme(source));
}

int QmlTool::cursorShape() const
{
    return mCursorShape;
}

void QmlTool::setCursorShape(int shape)
{
    mCursorShape = shape;
    setCursor(QCursor((Qt::CursorShape)shape));
}

QObject* QmlTool::options() const
{
    return mOptions;
}

void QmlTool::setOptions(QObject *opt)
{
    mOptions = opt;
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

void QmlTool::drawOverlay(QPainter *painter, const QRectF &)
{
    emit qmlDrawOverlay(painter);
}

void QmlTool::languageChanged()
{
}

}
