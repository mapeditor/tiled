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

#include "scriptmanager.h"

#include <QJSEngine>

namespace Tiled {

ScriptedAction::ScriptedAction(Id id,
                               const QJSValue &callback,
                               QObject *parent)
    : QAction(parent)
    , mId(id)
    , mCallback(callback)
{
    static QIcon scriptIcon = [] {
        QIcon icon(QStringLiteral("://images/32/plugin.png"));
        icon.addFile(QStringLiteral("://images/22/plugin.png"));
        icon.addFile(QStringLiteral("://images/16/plugin.png"));
        return icon;
    }();

    setIcon(scriptIcon);

    connect(this, &QAction::triggered, this, [this] {
        QJSValueList arguments;
        arguments.append(ScriptManager::instance().engine()->newQObject(this));

        QJSValue result = mCallback.call(arguments);
        ScriptManager::instance().checkError(result);
    });
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

} // namespace Tiled

#include "moc_scriptedaction.cpp"
