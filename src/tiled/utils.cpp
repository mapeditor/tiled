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


/**
 * Restores a widget's geometry.
 * Requires the widget to have its object name set.
 */
void restoreGeometry(QWidget *widget)
{
    Q_ASSERT(!widget->objectName().isEmpty());

    const QString key = widget->objectName() + QLatin1String("/Geometry");
    const QSettings *settings = Internal::Preferences::instance()->settings();
    widget->restoreGeometry(settings->value(key).toByteArray());
}

/**
 * Saves a widget's geometry.
 * Requires the widget to have its object name set.
 */
void saveGeometry(QWidget *widget)
{
    Q_ASSERT(!widget->objectName().isEmpty());

    const QString key = widget->objectName() + QLatin1String("/Geometry");
    QSettings *settings = Internal::Preferences::instance()->settings();
    settings->setValue(key, widget->saveGeometry());
}

/**
 * Takes a tileset name and splits it based on the '.'
 * We should have 2 or more '.' in the sentence to parse a valid name.
 * basically if we have 2 items we return the last item from the results of the split.
 * if it's greater than 2 we return (n-1) from the split result.
 * Also note that it returns nothing if the split results in less than 2 items.
 */
QString parsePreExtension(QString input)
{

    if (input.isNull() || input.isEmpty() || input.size() == 0) {
        return QString();
    }


    const QLatin1Char splitToken = QLatin1Char('.');
    const QStringList stringList = input.split(splitToken);
    const int itemCount = stringList.size();

    int tokenCount = 0;

    //count the number of tokens.
    for (int i = 0; i < input.size(); i++) {

        if (input.at(i) == splitToken) {
            tokenCount++;
        }
    }

    if (tokenCount < 1 || itemCount < 2) {
        return QString();
    }

    if (itemCount == 2) {
        return stringList.at(1);
    }
    return stringList.at(itemCount - 1);
}

} // namespace Utils
} // namespace Tiled
