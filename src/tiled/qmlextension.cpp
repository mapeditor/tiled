/*
 * qmlextension.cpp
 * Copyright 2026, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "qmlextension.h"

#include "actionmanager.h"
#include "logginginterface.h"
#include "pluginmanager.h"
#include "qmldock.h"
#include "scriptedfileformat.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QJSEngine>
#include <QQmlEngine>
#include <QTimer>

namespace Tiled {

QmlExtension::QmlExtension(QObject *parent)
    : QObject(parent)
{
}

void QmlExtension::setName(const QString &name)
{
    if (mName == name)
        return;

    mName = name;
    emit nameChanged();
}

QQmlListProperty<QObject> QmlExtension::data()
{
    return QQmlListProperty<QObject>(this, &mData);
}


QmlAction::QmlAction(QObject *parent)
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

QmlAction::~QmlAction()
{
    if (mRegistered)
        ActionManager::unregisterAction(this, mId);
}

QString QmlAction::name() const
{
    return mName;
}

void QmlAction::setName(const QString &name)
{
    mName = name;
}

void QmlAction::setIconFileName(const QString &fileName)
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

QString QmlAction::shortcutString() const
{
    return shortcut().toString();
}

void QmlAction::setShortcutString(const QString &shortcut)
{
    setShortcut(QKeySequence(shortcut));
}

void QmlAction::componentComplete()
{
    // May be called earlier by a QmlMenuExtension referencing this action
    if (mCompleted)
        return;

    mCompleted = true;

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


QmlMenuExtension::QmlMenuExtension(QObject *parent)
    : QObject(parent)
{
}

void QmlMenuExtension::componentComplete()
{
    // Defer the registration, since the order in which objects are completed
    // is not defined and the referenced actions may even be declared in
    // extension files that have not been loaded yet.
    QTimer::singleShot(0, this, &QmlMenuExtension::apply);
}

void QmlMenuExtension::apply()
{
    const Id menuId { mMenu.toUtf8() };

    if (mMenu.isEmpty() || !ActionManager::hasMenu(menuId)) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Unknown menu: '%1'").arg(mMenu));
        return;
    }

    ActionManager::MenuExtension extension;

    for (const QVariant &value : std::as_const(mItems)) {
        const QVariantMap map = value.toMap();
        ActionManager::MenuItem menuItem {};

        const QVariant action = map.value(QStringLiteral("action"));

        // An action can be referenced either by its ID or directly
        if (auto qmlAction = qobject_cast<QmlAction*>(action.value<QObject*>())) {
            menuItem.action = qmlAction->id();
        } else if (action.userType() == QMetaType::QString) {
            menuItem.action = Id(action.toString().toUtf8());
        }

        menuItem.beforeAction = Id(map.value(QStringLiteral("before")).toString().toUtf8());
        menuItem.isSeparator = map.value(QStringLiteral("separator")).toBool();

        if (!menuItem.action.isNull()) {
            if (menuItem.isSeparator) {
                Tiled::ERROR(QCoreApplication::translate("Script Errors", "Separators can't have actions"));
                return;
            }

            if (!ActionManager::findAction(menuItem.action)) {
                Tiled::ERROR(QCoreApplication::translate("Script Errors", "Unknown action: '%1'").arg(
                                 QString::fromUtf8(menuItem.action.name())));
                return;
            }
        } else if (!menuItem.isSeparator) {
            Tiled::ERROR(QCoreApplication::translate("Script Errors", "Non-separator item without action"));
            return;
        }

        extension.items.append(menuItem);
    }

    ActionManager::registerMenuExtension(menuId, extension);
}


QmlMapFormat::QmlMapFormat(QObject *parent)
    : QObject(parent)
{
}

QmlMapFormat::~QmlMapFormat()
{
}

void QmlMapFormat::componentComplete()
{
    if (mShortName.isEmpty()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "MapFormat without shortName"));
        return;
    }

    const auto formats = PluginManager::objects<MapFormat>();
    for (auto format : formats) {
        if (format->shortName() == mShortName) {
            Tiled::ERROR(QCoreApplication::translate("Script Errors", "Reserved shortName: '%1'").arg(mShortName));
            return;
        }
    }

    if (mName.isEmpty() || mExtension.isEmpty()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Invalid file format object (requires 'name' and 'extension' properties)"));
        return;
    }

    // This object is owned by the loaded extension, the garbage collector
    // should leave it alone.
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    const QJSValue self = ScriptManager::instance().engine()->newQObject(this);

    if (!self.property(QStringLiteral("read")).isCallable() &&
            !self.property(QStringLiteral("write")).isCallable()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Invalid file format object (requires a 'write' and/or 'read' function property)"));
        return;
    }

    mFormat = std::make_unique<ScriptedMapFormat>(mShortName, self, nullptr);
}


QmlTilesetFormat::QmlTilesetFormat(QObject *parent)
    : QObject(parent)
{
}

QmlTilesetFormat::~QmlTilesetFormat()
{
}

void QmlTilesetFormat::componentComplete()
{
    if (mShortName.isEmpty()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "TilesetFormat without shortName"));
        return;
    }

    const auto formats = PluginManager::objects<TilesetFormat>();
    for (auto format : formats) {
        if (format->shortName() == mShortName) {
            Tiled::ERROR(QCoreApplication::translate("Script Errors", "Reserved shortName: '%1'").arg(mShortName));
            return;
        }
    }

    if (mName.isEmpty() || mExtension.isEmpty()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Invalid file format object (requires 'name' and 'extension' properties)"));
        return;
    }

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    const QJSValue self = ScriptManager::instance().engine()->newQObject(this);

    if (!self.property(QStringLiteral("read")).isCallable() &&
            !self.property(QStringLiteral("write")).isCallable()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Invalid file format object (requires a 'write' and/or 'read' function property)"));
        return;
    }

    mFormat = std::make_unique<ScriptedTilesetFormat>(mShortName, self, nullptr);
}


QmlTool::QmlTool(QObject *parent)
    : ScriptedTool(parent)
{
}

void QmlTool::componentComplete()
{
    if (mShortName.isEmpty()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Tool without shortName"));
        return;
    }

    const Id id { mShortName.toUtf8() };
    const auto tools = PluginManager::objects<AbstractTool>();
    for (auto tool : tools) {
        if (tool->id() == id) {
            Tiled::ERROR(QCoreApplication::translate("Script Errors", "Reserved shortName: '%1'").arg(mShortName));
            return;
        }
    }

    setId(id);

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    setScriptObject(ScriptManager::instance().engine()->newQObject(this));
    addToPluginManager();
}


void registerQmlExtensionTypes()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    qmlRegisterType<QmlAction>("Tiled", 1, 0, "Action");
    qmlRegisterType<QmlDock>("Tiled", 1, 0, "Dock");
    qmlRegisterType<QmlExtension>("Tiled", 1, 0, "Extension");
    qmlRegisterType<QmlMapFormat>("Tiled", 1, 0, "MapFormat");
    qmlRegisterType<QmlMenuExtension>("Tiled", 1, 0, "MenuExtension");
    qmlRegisterType<QmlTilesetFormat>("Tiled", 1, 0, "TilesetFormat");
    qmlRegisterType<QmlTool>("Tiled", 1, 0, "Tool");
}

} // namespace Tiled

#include "moc_qmlextension.cpp"
