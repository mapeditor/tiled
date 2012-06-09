/*
 * utils.h
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

#ifndef UTILS_H
#define UTILS_H

#include <QIcon>
#include <QString>

class QAction;
class QMenu;

namespace Tiled {
namespace Utils {

/**
 * Returns a file dialog filter that matches all readable image formats.
 */
QString readableImageFormatsFilter();

/**
 * Returns a file dialog filter that matches all writable image formats.
 */
QString writableImageFormatsFilter();

/**
 * Looks up the icon with the specified \a name from the system theme and set
 * it on the instance \a t when found.
 *
 * This is a templated method which is used on instances of QAction, QMenu,
 * QToolButton, etc.
 *
 * Does nothing when the platform is not Linux.
 */
template <class T>
void setThemeIcon(T *t, const char *name)
{
#ifdef Q_OS_LINUX
    QIcon themeIcon = QIcon::fromTheme(QLatin1String(name));
    if (!themeIcon.isNull())
        t->setIcon(themeIcon);
#else
    Q_UNUSED(t)
    Q_UNUSED(name)
#endif
}

} // namespace Utils
} // namespace Tiled

#endif // UTILS_H
