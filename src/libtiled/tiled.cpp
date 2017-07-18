/*
 * tiled.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tiled.h"

#include <QDir>

QString Tiled::toFileReference(const QUrl &url, const QDir &dir)
{
    if (url.isEmpty())
        return QString();

    if (url.isLocalFile()) {
        // Local files are referred to by relative path
        QString localFile = url.toLocalFile();
        return dir.relativeFilePath(localFile);
    }

    return url.toString();
}

QUrl Tiled::toUrl(const QString &filePathOrUrl, const QDir &dir)
{
    if (filePathOrUrl.isEmpty())
        return QUrl();

    const QUrl url(filePathOrUrl, QUrl::StrictMode);
    if (!url.isRelative())
        return url;

    // Resolve possible relative file reference
    QString absolutePath = dir.filePath(filePathOrUrl);
    return QUrl::fromLocalFile(absolutePath);
}
