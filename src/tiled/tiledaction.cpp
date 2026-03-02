#include "tiledaction.h"
#include <QIcon>
#include <QKeySequence>

namespace Tiled {

QMLAction::QMLAction(QObject *parent)
    : QAction(parent)
{
    connect(this, &QAction::triggered,
            this, &TiledAction::triggeredFromQml);
}

QString QMLAction::id() const
{
    return mId;
}

void TQMLction::setId(const QString &id)
{
    mId = id;
    setObjectName(id);
}

QString QMLAction::iconSource() const
{
    return mIconSource;
}

void QMLAction::setIconSource(const QString &path)
{
    mIconSource = path;
    setIcon(QIcon(path));
}

QString QMLAction::shortcut() const
{
    return QAction::shortcut().toString();
}

void QMLAction::setShortcutString(const QString &shortcutStr)
{
    setShortcut(QKeySequence(shortcutStr));
}

}
