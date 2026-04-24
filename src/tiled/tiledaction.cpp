#include "tiledaction.h"

#include <QIcon>
#include <QKeySequence>

namespace Tiled {

QmlAction::QmlAction(QObject *parent)
    : QAction(parent)
{
    connect(this, &QAction::triggered,
            this, &QmlAction::triggeredFromQml);
}

QString QmlAction::id() const
{
    return mId;
}

void QmlAction::setId(const QString &id)
{
    mId = id;
    setObjectName(id);
}

QString QmlAction::iconSource() const
{
    return mIconSource;
}

void QmlAction::setIconSource(const QString &path)
{
    mIconSource = path;
    setIcon(QIcon(path));
}

QString QmlAction::shortcut() const
{
    return QAction::shortcut().toString();
}

void QmlAction::setShortcut(const QString &shortcutStr)
{
    QAction::setShortcut(QKeySequence(shortcutStr));
}

QString QmlAction::menu() const
{
    return mMenu;
}

void QmlAction::setMenu(const QString &menu)
{
    mMenu = menu;
}

QString QmlAction::context() const
{
    return mContext;
}

void QmlAction::setContext(const QString &context)
{
    mContext = context;
}

}

#include "moc_tiledaction.cpp"
