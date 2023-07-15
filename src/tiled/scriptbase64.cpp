/*
 * scriptbase64.cpp
 * Copyright 2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptbase64.h"

#include <QObject>
#include <QJSEngine>

namespace Tiled {

class ScriptBase64 : public QObject
{
    Q_OBJECT

public:
    ScriptBase64(QObject *parent = nullptr)
        : QObject(parent)
    {}

    Q_INVOKABLE QString encode(const QByteArray &data) const;
    Q_INVOKABLE QByteArray encodeAsBytes(const QByteArray &data) const;
    Q_INVOKABLE QByteArray decode(const QByteArray &data) const;
};

QString ScriptBase64::encode(const QByteArray &data) const
{
    return QString::fromLatin1(data.toBase64());
}

QByteArray ScriptBase64::encodeAsBytes(const QByteArray &data) const
{
    return data.toBase64();
}

QByteArray ScriptBase64::decode(const QByteArray &data) const
{
    return QByteArray::fromBase64(data);
}

void registerBase64(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("Base64"),
                                         jsEngine->newQObject(new ScriptBase64));

}

} // namespace Tiled

#include "scriptbase64.moc"
