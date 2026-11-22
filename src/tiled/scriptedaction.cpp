/*
 * scriptedaction.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scriptedaction.h"

#include "actionmanager.h"
#include "logginginterface.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QQmlEngine>

namespace Tiled {

ScriptedAction::ScriptedAction(QObject *parent)
    : QAction(parent)
{
    static QIcon scriptIcon = [] {
        QIcon icon(QStringLiteral("://images/32/plugin.png"));
        icon.addFile(QStringLiteral("://images/22/plugin.png"));
        icon.addFile(QStringLiteral("://images/16/plugin.png"));
        return icon;
    }();

    setIcon(scriptIcon);
}

ScriptedAction::ScriptedAction(Id id,
                               const QJSValue &callback,
                               QObject *parent)
    : ScriptedAction(parent)
{
    mId = id;
    mName = QString::fromUtf8(id.name());
    mCallback = callback;

    connect(this, &QAction::triggered, this, [this] {
        QJSValueList arguments;
        arguments.append(ScriptManager::instance().engine()->newQObject(this));

        QJSValue result = mCallback.call(arguments);
        ScriptManager::instance().checkError(result);
    });

    ActionManager::registerAction(this, id);
    mRegistered = true;
}

ScriptedAction::~ScriptedAction()
{
    if (mRegistered)
        ActionManager::unregisterAction(this, mId);
}

void ScriptedAction::setIconFileName(const QString &fileName)
{
    if (mIconFileName == fileName)
        return;

    mIconFileName = fileName;

    QString iconFile = fileName;

    const QString ext = QStringLiteral("ext:");
    if (!iconFile.startsWith(ext) && !iconFile.startsWith(QLatin1Char(':')))
        iconFile.prepend(ext);

    setIcon(QIcon { iconFile });
}

QString ScriptedAction::shortcutString() const
{
    return shortcut().toString();
}

void ScriptedAction::setShortcutString(const QString &shortcut)
{
    setShortcut(QKeySequence(shortcut));
}

void ScriptedAction::componentComplete()
{
    if (mName.isEmpty()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Action without name"));
        return;
    }

    const Id id { mName.toUtf8() };

    if (ActionManager::findAction(id)) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Reserved ID: '%1'").arg(mName));
        return;
    }

    mId = id;
    ActionManager::registerAction(this, id);
    mRegistered = true;
}

} // namespace Tiled

#include "moc_scriptedaction.cpp"
