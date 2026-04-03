/*
 * scriptsession.cpp
 * Copyright 2026, Kanishka
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

#include "scriptsession.h"

#include "session.h"

namespace Tiled {

ScriptSession::ScriptSession(QObject *parent)
    : QObject(parent)
{}

QString ScriptSession::fileName() const
{
    if (!Session::hasCurrent())
        return QString();
    return Session::current().fileName();
}

QVariant ScriptSession::get(const QString &key,
                            const QVariant &defaultValue) const
{
    if (!Session::hasCurrent())
        return defaultValue;

    const QByteArray latin1Key = key.toLatin1();
    auto &session = Session::current();

    return session.get<QVariant>(latin1Key.constData(), defaultValue);
}

void ScriptSession::set(const QString &key, const QVariant &value)
{
    if (!Session::hasCurrent())
        return;
    Session::current().set(key.toLatin1().constData(), value);
}

bool ScriptSession::isSet(const QString &key) const
{
    if (!Session::hasCurrent())
        return false;
    return Session::current().isSet(key.toLatin1().constData());
}

QVariantMap ScriptSession::fileState(const QString &fileName) const
{
    if (!Session::hasCurrent())
        return {};
    return Session::current().fileState(fileName);
}

void ScriptSession::setFileState(const QString &fileName,
                                 const QVariantMap &fileState)
{
    if (!Session::hasCurrent())
        return;
    Session::current().setFileState(fileName, fileState);
}

void ScriptSession::setFileStateValue(const QString &fileName,
                                      const QString &name,
                                      const QVariant &value)
{
    if (!Session::hasCurrent())
        return;
    Session::current().setFileStateValue(fileName, name, value);
}

} // namespace Tiled

#include "moc_scriptsession.cpp"
