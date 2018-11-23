/*
 * scriptmodule.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptmodule.h"

#include "actionmanager.h"

#include <QAction>
#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ScriptModule::ScriptModule(QObject *parent)
    : QObject(parent)
{
}

QString ScriptModule::version() const
{
    return QCoreApplication::applicationVersion();
}

QString ScriptModule::platform() const
{
#if defined(Q_OS_WIN)
    return QStringLiteral("windows");
#elif defined(Q_OS_MAC)
    return QStringLiteral("macos");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("linux");
#else
    return QStringLiteral("unix");
#endif
}

QString ScriptModule::arch() const
{
#if defined(Q_PROCESSOR_X86_64)
    return QStringLiteral("x64");
#elif defined(Q_PROCESSOR_X86)
    return QStringLiteral("x86");
#else
    return QStringLiteral("unknown");
#endif
}

DocumentManager *ScriptModule::documentManager() const
{
    return DocumentManager::instance();
}

void ScriptModule::trigger(const QByteArray &actionName)
{
    if (QAction *action = ActionManager::findAction(Id(actionName)))
        action->trigger();
}

} // namespace Internal
} // namespace Tiled
