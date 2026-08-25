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

#include <QObject>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Tiled {

/**
 * Groups any number of extension objects, giving the extension a name.
 *
 * Purely a container for now, but reserved to enable dynamically
 * enabling/disabling extensions in the UI later on.
 */
class QmlExtension : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Extension)

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
    QML_NAMED_ELEMENT(MenuExtension)

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

} // namespace Tiled
