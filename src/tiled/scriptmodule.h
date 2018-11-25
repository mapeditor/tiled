/*
 * scriptmodule.h
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

#pragma once

#include "documentmanager.h"

#include <QObject>

namespace Tiled {
namespace Internal {

/**
 * Initial point of access to Tiled functionality from JavaScript.
 */
class ScriptModule : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString version READ version)
    Q_PROPERTY(QString platform READ platform)
    Q_PROPERTY(QString arch READ arch)

    Q_PROPERTY(DocumentManager *documentManager READ documentManager)

public:
    ScriptModule(QObject *parent = nullptr);

    QString version() const;
    QString platform() const;
    QString arch() const;

    DocumentManager *documentManager() const;

public slots:
    void trigger(const QByteArray &actionName);

    void alert(const QString &text, const QString &title = QString());
    bool confirm(const QString &text, const QString &title = QString());
    QString prompt(const QString &label, const QString &text = QString(), const QString &title = QString());
};

} // namespace Internal
} // namespace Tiled
