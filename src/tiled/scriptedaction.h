/*
 * scriptedaction.h
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

#pragma once

#include "id.h"

#include <QAction>
#include <QJSValue>
#include <QQmlParserStatus>
#include <QtQml/qqmlregistration.h>

namespace Tiled {

/**
 * An action registered by an extension, either through
 * tiled.registerAction or declared as a QML component. It registers itself
 * with the ActionManager and unregisters on destruction.
 */
class ScriptedAction : public QAction, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_NAMED_ELEMENT(Action)

    Q_PROPERTY(QByteArray id READ idName CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString icon READ iconFileName WRITE setIconFileName)
    Q_PROPERTY(QString shortcut READ shortcutString WRITE setShortcutString)

public:
    explicit ScriptedAction(QObject *parent = nullptr);
    ScriptedAction(Id id,
                   const QJSValue &callback,
                   QObject *parent = nullptr);
    ~ScriptedAction() override;

    Id id() const;
    QByteArray idName() const;

    QString name() const;
    void setName(const QString &name);

    QString iconFileName() const;
    void setIconFileName(const QString &fileName);

    // Shadows QAction::shortcut to support assigning a string in QML
    QString shortcutString() const;
    void setShortcutString(const QString &shortcut);

    void classBegin() override {}
    void componentComplete() override;

private:
    Id mId;
    QString mName;
    QJSValue mCallback;
    QString mIconFileName;
    bool mRegistered = false;
};


inline Id ScriptedAction::id() const
{
    return mId;
}

inline QByteArray ScriptedAction::idName() const
{
    return mId.name();
}

inline QString ScriptedAction::name() const
{
    return mName;
}

inline void ScriptedAction::setName(const QString &name)
{
    mName = name;
}

inline QString ScriptedAction::iconFileName() const
{
    return mIconFileName;
}

} // namespace Tiled
