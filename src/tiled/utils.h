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

#pragma once

#include "rangeset.h"

#include <QIcon>
#include <QJsonParseError>
#include <QSettings>
#include <QString>

#include <memory>

#include "qtcompat_p.h"

class QAction;
class QKeyEvent;
class QMenu;

namespace Tiled {
namespace Utils {

QString readableImageFormatsFilter();
QString writableImageFormatsFilter();

QStringList cleanFilterList(const QString &filter);
bool fileNameMatchesNameFilter(const QString &fileName,
                               const QString &nameFilter);
QString firstExtension(const QString &nameFilter);

int matchingScore(const QStringList &words, QStringRef string);
RangeSet<int> matchingRanges(const QStringList &words, QStringRef string);

QIcon themeIcon(const QString &name);

/**
 * Looks up the icon with the specified \a name from the system theme and set
 * it on the instance \a t when found.
 *
 * This is a template method which is used on instances of QAction, QMenu,
 * QToolButton, etc.
 *
 * Does nothing when the platform is not Linux.
 */
template <class T>
void setThemeIcon(T *t, const QString &name)
{
#ifdef Q_OS_LINUX
    const QIcon icon = themeIcon(name);
    if (!icon.isNull())
        t->setIcon(icon);
#else
    Q_UNUSED(t)
    Q_UNUSED(name)
#endif
}

template <class T>
void setThemeIcon(T *t, const char *name)
{
    setThemeIcon(t, QLatin1String(name));
}

QIcon colorIcon(const QColor &color, QSize size);

QRect screenRect(const QWidget *widget);
QRect popupGeometry(const QWidget *parent, QSize popupSize);

void restoreGeometry(QWidget *widget);
void saveGeometry(QWidget *widget);

int defaultDpi();
qreal defaultDpiScale();
int dpiScaled(int value);
qreal dpiScaled(qreal value);
QSize dpiScaled(QSize value);
QPoint dpiScaled(QPoint value);
QRectF dpiScaled(QRectF value);
QSize smallIconSize();

bool isZoomInShortcut(QKeyEvent *event);
bool isZoomOutShortcut(QKeyEvent *event);
bool isResetZoomShortcut(QKeyEvent *event);

void addFileManagerActions(QMenu &menu, const QString &fileName);
void addOpenContainingFolderAction(QMenu &menu, const QString &fileName);
void addOpenWithSystemEditorAction(QMenu &menu, const QString &fileName);

QSettings::Format jsonSettingsFormat();
std::unique_ptr<QSettings> jsonSettings(const QString &fileName);

namespace Error {
QString jsonParseError(QJsonParseError error);
} // namespace Error

} // namespace Utils
} // namespace Tiled
