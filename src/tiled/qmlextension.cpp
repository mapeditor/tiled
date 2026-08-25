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
#include "qmldock.h"
#include "scriptedaction.h"
#include "scriptedfileformat.h"
#include "scriptedtool.h"

#include <QCoreApplication>
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
        if (auto scriptedAction = qobject_cast<ScriptedAction*>(action.value<QObject*>())) {
            menuItem.action = scriptedAction->id();
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


void registerQmlExtensionTypes()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    qmlRegisterType<ScriptedAction>("Tiled", 1, 0, "Action");
    qmlRegisterType<QmlDock>("Tiled", 1, 0, "Dock");
    qmlRegisterType<QmlExtension>("Tiled", 1, 0, "Extension");
    qmlRegisterType<ScriptedMapFormat>("Tiled", 1, 0, "MapFormat");
    qmlRegisterType<QmlMenuExtension>("Tiled", 1, 0, "MenuExtension");
    qmlRegisterType<ScriptedTilesetFormat>("Tiled", 1, 0, "TilesetFormat");
    qmlRegisterType<ScriptedTool>("Tiled", 1, 0, "Tool");
}

} // namespace Tiled

#include "moc_qmlextension.cpp"
