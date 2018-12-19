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
#include "logginginterface.h"

#include <QAction>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMessageBox>

namespace Tiled {

ScriptModule::ScriptModule(QObject *parent)
    : QObject(parent)
    , mLogger(new LoggingInterface(this))
{
    auto documentManager = DocumentManager::instance();
    connect(documentManager, &DocumentManager::documentCreated, this, &ScriptModule::documentCreated);
    connect(documentManager, &DocumentManager::documentOpened, this, &ScriptModule::documentOpened);
    connect(documentManager, &DocumentManager::documentAboutToBeSaved, this, &ScriptModule::documentAboutToBeSaved);
    connect(documentManager, &DocumentManager::documentSaved, this, &ScriptModule::documentSaved);
    connect(documentManager, &DocumentManager::documentAboutToClose, this, &ScriptModule::documentAboutToClose);
    connect(documentManager, &DocumentManager::currentDocumentChanged, this, &ScriptModule::currentDocumentChanged);

    PluginManager::addObject(mLogger);
}

ScriptModule::~ScriptModule()
{
    PluginManager::removeObject(mLogger);
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

EditableAsset *ScriptModule::activeAsset() const
{
    auto documentManager = DocumentManager::instance();
    if (Document *document = documentManager->currentDocument())
        return document->editable();

    return nullptr;
}

bool ScriptModule::setActiveAsset(EditableAsset *asset) const
{
    auto documentManager = DocumentManager::instance();
    for (const DocumentPtr &document : documentManager->documents())
        if (document->editable() == asset)
            return documentManager->switchToDocument(document.data());

    return false;
}

QList<QObject *> ScriptModule::openAssets() const
{
    auto documentManager = DocumentManager::instance();
    QList<QObject *> assets;
    for (const DocumentPtr &document : documentManager->documents())
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

void ScriptModule::log(const QString &text) const
{
    mLogger->info(text);
}

void ScriptModule::error(const QString &text) const
{
    mLogger->error(tr("Error: %1").arg(text));
}

void ScriptModule::documentCreated(Document *document)
{
    emit assetCreated(document->editable());
}

void ScriptModule::documentOpened(Document *document)
{
    emit assetOpened(document->editable());
}

void ScriptModule::documentAboutToBeSaved(Document *document)
{
    emit assetAboutToBeSaved(document->editable());
}

void ScriptModule::documentSaved(Document *document)
{
    emit assetSaved(document->editable());
}

void ScriptModule::documentAboutToClose(Document *document)
{
    emit assetAboutToBeClosed(document->editable());
}

void ScriptModule::currentDocumentChanged(Document *document)
{
    emit activeAssetChanged(document ? document->editable() : nullptr);
}

} // namespace Tiled
