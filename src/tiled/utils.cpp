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

#include "preferences.h"

#include <QAction>
#include <QCoreApplication>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QMainWindow>
#include <QMenu>
#include <QRegExp>
#include <QScreen>
#include <QSettings>

static QString toImageFileFilter(const QList<QByteArray> &formats)
{
    QString filter(QCoreApplication::translate("Utils", "Image files"));
    filter += QLatin1String(" (");
    bool first = true;
    for (const QByteArray &format : formats) {
        if (!first)
            filter += QLatin1Char(' ');
        first = false;
        filter += QLatin1String("*.");
        filter += QString::fromLatin1(format.toLower());
    }
    filter += QLatin1Char(')');
    return filter;
}

// Makes a list of filters from a normal filter string "Image Files (*.png *.jpg)"
//
// Copied from qplatformdialoghelper.cpp in Qt, used under the terms of the GPL
// version 2.0.
static QStringList cleanFilterList(const QString &filter)
{
    const char filterRegExp[] =
    "^(.*)\\(([a-zA-Z0-9_.,*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

    QRegExp regexp(QString::fromLatin1(filterRegExp));
    Q_ASSERT(regexp.isValid());
    QString f = filter;
    int i = regexp.indexIn(f);
    if (i >= 0)
        f = regexp.cap(2);
    return f.split(QLatin1Char(' '), QString::SkipEmptyParts);
}

namespace Tiled {
namespace Utils {

/**
 * Returns a file dialog filter that matches all readable image formats.
 */
QString readableImageFormatsFilter()
{
    return toImageFileFilter(QImageReader::supportedImageFormats());
}

/**
 * Returns a file dialog filter that matches all writable image formats.
 */
QString writableImageFormatsFilter()
{
    return toImageFileFilter(QImageWriter::supportedImageFormats());
}

/**
 * Returns whether the \a fileName has an extension that is matched by
 * the \a nameFilter.
 */
bool fileNameMatchesNameFilter(const QString &fileName,
                               const QString &nameFilter)
{
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPatternSyntax(QRegExp::Wildcard);

    const QStringList filterList = cleanFilterList(nameFilter);
    for (const QString &filter : filterList) {
        rx.setPattern(filter);
        if (rx.exactMatch(fileName))
            return true;
    }
    return false;
}


/**
 * Restores a widget's geometry.
 * Requires the widget to have its object name set.
 */
void restoreGeometry(QWidget *widget)
{
    Q_ASSERT(!widget->objectName().isEmpty());

    const QSettings *settings = Internal::Preferences::instance()->settings();

    const QString key = widget->objectName() + QLatin1String("/Geometry");
    widget->restoreGeometry(settings->value(key).toByteArray());

    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(widget)) {
        const QString stateKey = widget->objectName() + QLatin1String("/State");
        mainWindow->restoreState(settings->value(stateKey).toByteArray());
    }
}

/**
 * Saves a widget's geometry.
 * Requires the widget to have its object name set.
 */
void saveGeometry(QWidget *widget)
{
    Q_ASSERT(!widget->objectName().isEmpty());

    QSettings *settings = Internal::Preferences::instance()->settings();

    const QString key = widget->objectName() + QLatin1String("/Geometry");
    settings->setValue(key, widget->saveGeometry());

    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(widget)) {
        const QString stateKey = widget->objectName() + QLatin1String("/State");
        settings->setValue(stateKey, mainWindow->saveState());
    }
}

qreal defaultDpiScale()
{
    static qreal scale = []() {
        if (const QScreen *screen = QGuiApplication::primaryScreen())
            return screen->logicalDotsPerInchX() / 96.0;
        return 1.0;
    }();
    return scale;
}

qreal dpiScaled(qreal value)
{
#ifdef Q_OS_MAC
    // On mac the DPI is always 72 so we should not scale it
    return value;
#else
    static const qreal scale = defaultDpiScale();
    return value * scale;
#endif
}

QSize dpiScaled(QSize value)
{
    return QSize(qRound(dpiScaled(value.width())),
                 qRound(dpiScaled(value.height())));
}

QPoint dpiScaled(QPoint value)
{
    return QPoint(qRound(dpiScaled(value.x())),
                  qRound(dpiScaled(value.y())));
}

QRectF dpiScaled(QRectF value)
{
    return QRectF(dpiScaled(value.x()),
                  dpiScaled(value.y()),
                  dpiScaled(value.width()),
                  dpiScaled(value.height()));
}

QSize smallIconSize()
{
    static QSize size = dpiScaled(QSize(16, 16));
    return size;
}

} // namespace Utils
} // namespace Tiled
