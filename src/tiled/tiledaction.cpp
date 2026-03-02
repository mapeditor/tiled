#include "tiledaction.h"
#include <QIcon>
#include <QKeySequence>

namespace Tiled {

TiledAction::TiledAction(QObject *parent)
    : QAction(parent)
{
    connect(this, &QAction::triggered,
            this, &TiledAction::triggeredFromQml);
}

QString TiledAction::id() const
{
    return mId;
}

void TiledAction::setId(const QString &id)
{
    mId = id;
    setObjectName(id);
}

QString TiledAction::iconSource() const
{
    return mIconSource;
}

void TiledAction::setIconSource(const QString &path)
{
    mIconSource = path;
    setIcon(QIcon(path));
}

QString TiledAction::shortcut() const
{
    return QAction::shortcut().toString();
}

void TiledAction::setShortcutString(const QString &shortcutStr)
{
    setShortcut(QKeySequence(shortcutStr));
}

}
