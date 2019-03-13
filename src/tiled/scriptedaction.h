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

namespace Tiled {

class ScriptedAction : public QAction
{
    Q_OBJECT

    Q_PROPERTY(QByteArray id READ idName CONSTANT)
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName)

public:
    ScriptedAction(Id idName,
                   const QJSValue &callback,
                   QObject *parent = nullptr);

    Id id() const;
    QByteArray idName() const;

    QString iconName() const;
    void setIconName(const QString &name);

private:
    Id mId;
    QJSValue mCallback;
    QString mIconName;
};


inline Id ScriptedAction::id() const
{
    return mId;
}

inline QByteArray ScriptedAction::idName() const
{
    return mId.name();
}

inline QString ScriptedAction::iconName() const
{
    return mIconName;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptedAction*)
