/*
 * textobjectfontwarninghelper.cpp
 * Copyright 2026, Tiled contributors
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

#include "textobjectfontwarninghelper.h"

#include "mapobject.h"

#include <QFontDatabase>

namespace Tiled {

static bool isGenericFontFamily(const QString &family)
{
    static const QSet<QString> genericFamilies {
        QStringLiteral("sans-serif"),
        QStringLiteral("serif"),
        QStringLiteral("monospace"),
        QStringLiteral("cursive"),
        QStringLiteral("fantasy"),
        QStringLiteral("system-ui"),
        QStringLiteral("ui-serif"),
        QStringLiteral("ui-sans-serif"),
        QStringLiteral("ui-monospace"),
        QStringLiteral("ui-rounded"),
        QStringLiteral("emoji"),
        QStringLiteral("math"),
        QStringLiteral("fangsong"),
    };

    return genericFamilies.contains(family.toCaseFolded());
}

QStringList availableFontFamilies()
{
    return QFontDatabase().families();
}

QString missingTextObjectFontFamily(const MapObject &mapObject,
                                    const QStringList &availableFamilies)
{
    if (mapObject.shape() != MapObject::Text)
        return {};

    const QString family = mapObject.textData().font.family().trimmed();
    if (family.isEmpty() || isGenericFontFamily(family))
        return {};

    return availableFamilies.contains(family, Qt::CaseInsensitive) ? QString() : family;
}

} // namespace Tiled
