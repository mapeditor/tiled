/*
 * editor.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "editor.h"

#include <QCoreApplication>
#include <QRegularExpression>

namespace Tiled {

Preference<bool> Editor::duplicateAddsCopy { "Editor/DuplicateAddsCopy" };

Editor::Editor(QObject *parent)
    : QObject(parent)
{
}

QString Editor::nameOfDuplicate(const QString &name)
{
    if (name.isEmpty() || !duplicateAddsCopy)
        return name;

    const QString copyText = tr("Copy");

    // Look for an existing postfix, optionally capturing a number
    const QRegularExpression regexp = QRegularExpression(QStringLiteral(
        R"((.*)\s*%1\s*(\d+)?$)").arg(copyText));

    const QRegularExpressionMatch match = regexp.match(name);
    if (match.hasMatch()) {
        const QString copyName = match.captured(1).trimmed();
        const QString capturedNumber = match.captured(2);
        const int copyNumber = capturedNumber.isNull() ? 2 : capturedNumber.toInt() + 1;

        return QStringLiteral("%1 %2 %3").arg(copyName, copyText,
                                              QString::number(copyNumber));
    }

    return QStringLiteral("%1 %2").arg(name, copyText);
}

} // namespace Tiled

#include "moc_editor.cpp"
