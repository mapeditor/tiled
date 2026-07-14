/*
 * qmlextension.h
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

#pragma once

#include "id.h"
#include "scriptedtool.h"

#include <QAction>
#include <QObject>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QVariantList>

#include <memory>

namespace Tiled {

class ScriptedMapFormat;
class ScriptedTilesetFormat;

/**
 * Groups any number of extension objects, giving the extension a name.
 *
 * Purely a container for now, but reserved to enable dynamically
 * enabling/disabling extensions in the UI later on.
 */
class QmlExtension : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_CLASSINFO("DefaultProperty", "data")

public:
    explicit QmlExtension(QObject *parent = nullptr);

    QString name() const { return mName; }
    void setName(const QString &name);

    QQmlListProperty<QObject> data();

signals:
    void nameChanged();

private:
    QString mName;
    QList<QObject*> mData;
};

/**
 * A declaratively registered action, which registers itself with the
 * ActionManager once its declared properties are set and unregisters on
 * destruction (when the extension is unloaded or the engine is reset).
 */
class QmlAction : public QAction, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString icon READ iconFileName WRITE setIconFileName)
    Q_PROPERTY(QString shortcut READ shortcutString WRITE setShortcutString)

public:
    explicit QmlAction(QObject *parent = nullptr);
    ~QmlAction() override;

    QString name() const;
    void setName(const QString &name);

    Id id() const { return mId; }

    QString iconFileName() const { return mIconFileName; }
    void setIconFileName(const QString &fileName);

    // Shadows QAction::shortcut to support assigning a string in QML
    QString shortcutString() const;
    void setShortcutString(const QString &shortcut);

    void classBegin() override {}
    void componentComplete() override;

private:
    QString mName;
    QString mIconFileName;
    Id mId;
    bool mCompleted = false;
    bool mRegistered = false;
};

/**
 * Extends a registered menu with additional items, mirroring
 * tiled.extendMenu from the JavaScript API.
 *
 * Each entry in \a items is an object with the following properties:
 *
 * - action: the ID of a registered action, or a reference to an Action
 * - before: the ID of an action before which to insert this item
 * - separator: whether this item is a separator
 */
class QmlMenuExtension : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString menu READ menu WRITE setMenu)
    Q_PROPERTY(QVariantList items READ items WRITE setItems)

public:
    explicit QmlMenuExtension(QObject *parent = nullptr);

    QString menu() const { return mMenu; }
    void setMenu(const QString &menu) { mMenu = menu; }

    QVariantList items() const { return mItems; }
    void setItems(const QVariantList &items) { mItems = items; }

    void classBegin() override {}
    void componentComplete() override;

private:
    void apply();

    QString mMenu;
    QVariantList mItems;
};

/**
 * A declaratively registered map format. The 'read', 'write' and
 * 'outputFiles' functions can be declared as QML functions on this object.
 */
class QmlMapFormat : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString shortName READ shortName WRITE setShortName)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString extension READ extension WRITE setExtension)

public:
    explicit QmlMapFormat(QObject *parent = nullptr);
    ~QmlMapFormat() override;

    QString shortName() const { return mShortName; }
    void setShortName(const QString &shortName) { mShortName = shortName; }

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QString extension() const { return mExtension; }
    void setExtension(const QString &extension) { mExtension = extension; }

    void classBegin() override {}
    void componentComplete() override;

private:
    QString mShortName;
    QString mName;
    QString mExtension;
    std::unique_ptr<ScriptedMapFormat> mFormat;
};

/**
 * A declaratively registered tileset format. The 'read' and 'write'
 * functions can be declared as QML functions on this object.
 */
class QmlTilesetFormat : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString shortName READ shortName WRITE setShortName)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString extension READ extension WRITE setExtension)

public:
    explicit QmlTilesetFormat(QObject *parent = nullptr);
    ~QmlTilesetFormat() override;

    QString shortName() const { return mShortName; }
    void setShortName(const QString &shortName) { mShortName = shortName; }

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QString extension() const { return mExtension; }
    void setExtension(const QString &extension) { mExtension = extension; }

    void classBegin() override {}
    void componentComplete() override;

private:
    QString mShortName;
    QString mName;
    QString mExtension;
    std::unique_ptr<ScriptedTilesetFormat> mFormat;
};

/**
 * A declaratively registered tool. Event handlers like 'activated',
 * 'mousePressed' or 'tilePositionChanged' can be declared as QML functions
 * on this object, like they are declared on the object passed to
 * tiled.registerTool in the JavaScript API.
 */
class QmlTool : public ScriptedTool, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString shortName READ shortName WRITE setShortName)

public:
    explicit QmlTool(QObject *parent = nullptr);

    QString shortName() const { return mShortName; }
    void setShortName(const QString &shortName) { mShortName = shortName; }

    void classBegin() override {}
    void componentComplete() override;

private:
    QString mShortName;
};

/**
 * Registers the QML types provided by the "Tiled" import. Safe to call
 * multiple times; registration only happens once per process.
 */
void registerQmlExtensionTypes();

} // namespace Tiled
