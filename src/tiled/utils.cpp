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
#include <QSettings>
#include <QImageReader>
#include <QImageWriter>
#include <QMainWindow>
#include <QMenu>

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

} // namespace Utils
} // namespace Tiled
