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
#include "scriptedaction.h"
#include "scriptedmapformat.h"
#include "scriptmanager.h"

#include <QAction>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMenu>
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

    for (const auto &pair : mRegisteredActions)
        ActionManager::unregisterAction(pair.second->id());

    for (const auto &pair : mRegisteredMapFormats)
        PluginManager::removeObject(pair.second.get());
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

static QStringList idsToNames(const QList<Id> &ids)
{
    QStringList names;
    for (const Id &id : ids)
        names.append(QLatin1String(id.name()));

    names.sort();

    return names;
}

QStringList ScriptModule::actions() const
{
    return idsToNames(ActionManager::actions());
}

QStringList ScriptModule::menus() const
{
    return idsToNames(ActionManager::menus());
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

ScriptedAction *ScriptModule::registerAction(const QByteArray &idName, QJSValue callback)
{
    if (idName.isEmpty()) {
        ScriptManager::instance().throwError(tr("Invalid ID"));
        return nullptr;
    }

    if (!callback.isCallable()) {
        ScriptManager::instance().throwError(tr("Invalid callback function"));
        return nullptr;
    }

    Id id(idName);
    auto &action = mRegisteredActions[idName];

    // Remove any previously registered action with the same name
    if (action) {
        ActionManager::unregisterAction(id);
    } else if (ActionManager::findAction(id)) {
        ScriptManager::instance().throwError(tr("Reserved ID"));
        return nullptr;
    }

    action.reset(new ScriptedAction(id, callback, this));
    ActionManager::registerAction(action.get(), id);
    return action.get();
}

void ScriptModule::registerMapFormat(const QString &shortName, QJSValue mapFormatObject)
{
    if (shortName.isEmpty()) {
        ScriptManager::instance().throwError(tr("Invalid shortName"));
        return;
    }

    if (!ScriptedMapFormat::validateMapFormatObject(mapFormatObject))
        return;

    auto &format = mRegisteredMapFormats[shortName];

    // Remove any previously registered format with the same name
    if (format)
        PluginManager::removeObject(format.get());

    format.reset(new ScriptedMapFormat(shortName, mapFormatObject));
    PluginManager::addObject(format.get());
}

static QString toString(QJSValue value)
{
    if (value.isString())
        return value.toString();
    return QString();
}

static Id toId(QJSValue value)
{
    return Id(toString(value).toUtf8());
}

void ScriptModule::extendMenu(const QByteArray &idName, QJSValue items)
{
    MenuExtension extension;
    extension.menuId = Id(idName);

    if (!ActionManager::findMenu(extension.menuId)) {
        ScriptManager::instance().throwError(tr("Unknown menu"));
        return;
    }

    auto addItem = [&] (QJSValue item) -> bool {
        MenuItem menuItem;

        const QJSValue action = item.property(QStringLiteral("action"));
        const QJSValue text = item.property(QStringLiteral("text"));

        menuItem.action = toId(action);
        menuItem.beforeAction = toId(item.property(QStringLiteral("before")));
        menuItem.isSeparator = item.property(QStringLiteral("separator")).toBool();

        if (!menuItem.action.isNull()) {
            if (menuItem.isSeparator) {
                ScriptManager::instance().throwError(tr("Separators can't have actions"));
                return false;
            }

            if (!ActionManager::findAction(menuItem.action)) {
                ScriptManager::instance().throwError(tr("Unknown action: '%1'").arg(
                                                         QString::fromUtf8(menuItem.action.name())));
                return false;
            }
        } else if (!menuItem.isSeparator) {
            ScriptManager::instance().throwError(tr("Non-separator item without action"));
            return false;
        }

        extension.items.append(menuItem);
        return true;
    };

    // Support either a single menu item or an array of items
    if (items.isArray()) {
        const quint32 length = items.property(QStringLiteral("length")).toUInt();
        for (quint32 i = 0; i < length; ++i)
            if (!addItem(items.property(i)))
                return;
    } else if (!addItem(items)) {
        return;
    }

    // Apply the extension
    QMenu *menu = ActionManager::menu(extension.menuId);
    QAction *before = nullptr;

    for (MenuItem &item : extension.items) {
        if (item.beforeAction)
            before = ActionManager::findAction(item.beforeAction);

        if (item.isSeparator)
            menu->insertSeparator(before)->setParent(this);
        else
            menu->insertAction(before, ActionManager::action(item.action));
    }

    mMenuExtensions.append(extension);
}

void ScriptModule::trigger(const QByteArray &actionName) const
{
    if (QAction *action = ActionManager::findAction(actionName))
        action->trigger();
    else
        ScriptManager::instance().throwError(tr("Unknown action"));
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
