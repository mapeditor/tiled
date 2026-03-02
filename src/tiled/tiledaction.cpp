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

void Qmlction::setId(const QString &id)
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

void QmlAction::setShortcutString(const QString &shortcutStr)
{
    setShortcut(QKeySequence(shortcutStr));
}

}
