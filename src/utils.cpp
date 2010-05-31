/*
 * utils.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "utils.h"

#include <QAction>
#include <QCoreApplication>
#include <QIcon>
#include <QImageReader>
#include <QImageWriter>
#include <QMenu>

static QString toImageFileFilter(const QList<QByteArray> &formats)
{
    QString filter(QCoreApplication::translate("Utils", "Image files"));
    filter += QLatin1String(" (");
    bool first = true;
    foreach (const QByteArray &format, formats) {
        if (!first)
            filter += QLatin1Char(' ');
        first = false;
        filter += QLatin1String("*.");
        filter += QString::fromLatin1(format.toLower());
    }
    filter += QLatin1Char(')');
    return filter;
}

namespace Tiled {
namespace Utils {

QString readableImageFormatsFilter()
{
    return toImageFileFilter(QImageReader::supportedImageFormats());
}

QString writableImageFormatsFilter()
{
    return toImageFileFilter(QImageWriter::supportedImageFormats());
}

void setThemeIcon(QAction *action, const char *name)
{
#if QT_VERSION >= 0x040600 && defined(Q_OS_LINUX)
    QIcon themeIcon = QIcon::fromTheme(QLatin1String(name));
    if (!themeIcon.isNull())
        action->setIcon(themeIcon);
#else
    Q_UNUSED(action)
    Q_UNUSED(name)
#endif
}

void setThemeIcon(QMenu *menu, const char *name)
{
#if QT_VERSION >= 0x040600 && defined(Q_OS_LINUX)
    QIcon themeIcon = QIcon::fromTheme(QLatin1String(name));
    if (!themeIcon.isNull())
        menu->setIcon(themeIcon);
#else
    Q_UNUSED(menu)
    Q_UNUSED(name)
#endif
}

} // namespace Utils
} // namespace Tiled
