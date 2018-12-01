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
#include "editableasset.h"

#include <QAction>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMessageBox>

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

EditableAsset *ScriptModule::activeAsset() const
{
    if (Document *document = documentManager()->currentDocument())
        return document->editable();

    return nullptr;
}

QList<QObject *> ScriptModule::openAssets() const
{
    QList<QObject *> assets;
    for (const DocumentPtr &document : documentManager()->documents())
        assets.append(document->editable());
    return assets;
}

void ScriptModule::trigger(const QByteArray &actionName) const
{
    if (QAction *action = ActionManager::findAction(Id(actionName)))
        action->trigger();
}

void ScriptModule::alert(const QString &text, const QString &title) const
{
    QMessageBox::warning(nullptr, title, text);
}

bool ScriptModule::confirm(const QString &text, const QString &title) const
{
    return QMessageBox::question(nullptr, title, text) == QMessageBox::Yes;
}

QString ScriptModule::prompt(const QString &label, const QString &text, const QString &title) const
{
    return QInputDialog::getText(nullptr, title, label, QLineEdit::Normal, text);
}

} // namespace Internal
} // namespace Tiled
