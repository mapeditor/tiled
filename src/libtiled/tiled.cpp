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

QPointF Tiled::alignmentOffset(const QRectF &r, Tiled::Alignment alignment)
{
    switch (alignment) {
    case Unspecified:   break;
    case TopLeft:       break;
    case Top:           return QPointF(r.width() / 2, 0);
    case TopRight:      return QPointF(r.width(), 0);
    case Left:          return QPointF(0, r.height() / 2);
    case Center:        return QPointF(r.width() / 2, r.height() / 2);
    case Right:         return QPointF(r.width(), r.height() / 2);
    case BottomLeft:    return QPointF(0, r.height());
    case Bottom:        return QPointF(r.width() / 2, r.height());
    case BottomRight:   return QPointF(r.width(), r.height());
    }
    return QPointF();
}

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

    // What appears to be a relative path may be a non-relative URL
    if (QDir::isRelativePath(filePathOrUrl)) {
        const QUrl url(filePathOrUrl, QUrl::StrictMode);
        if (!url.isRelative())
            return url;
    }

    // Resolve possible relative file reference
    QString absolutePath = QDir::cleanPath(dir.filePath(filePathOrUrl));
    if (absolutePath.startsWith(QLatin1String(":/")))
        return QUrl(QLatin1String("qrc") + absolutePath);

    return QUrl::fromLocalFile(absolutePath);
}

/*
    If \a url is a local file returns a path suitable for passing to QFile.
    Otherwise returns an empty string.
*/
QString Tiled::urlToLocalFileOrQrc(const QUrl &url)
{
    if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
        if (url.authority().isEmpty())
            return QLatin1Char(':') + url.path();
        return QString();
    }

    if (url.scheme() == QLatin1String("ext"))
        return url.toString();

#if defined(Q_OS_ANDROID)
    if (url.scheme().compare(QLatin1String("assets"), Qt::CaseInsensitive) == 0) {
        if (url.authority().isEmpty())
            return url.toString();
        return QString();
    }
#endif

    return url.toLocalFile();
}

QString Tiled::alignmentToString(Alignment alignment)
{
    switch (alignment) {
    case Unspecified:
        return QStringLiteral("unspecified");
    case TopLeft:
        return QStringLiteral("topleft");
    case Top:
        return QStringLiteral("top");
    case TopRight:
        return QStringLiteral("topright");
    case Left:
        return QStringLiteral("left");
    case Center:
        return QStringLiteral("center");
    case Right:
        return QStringLiteral("right");
    case BottomLeft:
        return QStringLiteral("bottomleft");
    case Bottom:
        return QStringLiteral("bottom");
    case BottomRight:
        return QStringLiteral("bottomright");
    }
    return QString();
}

Tiled::Alignment Tiled::alignmentFromString(const QString &string)
{
    if (string == QLatin1String("unspecified"))
        return Unspecified;
    else if (string == QLatin1String("topleft"))
        return TopLeft;
    else if (string == QLatin1String("top"))
        return Top;
    else if (string == QLatin1String("topright"))
        return TopRight;
    else if (string == QLatin1String("left"))
        return Left;
    else if (string == QLatin1String("center"))
        return Center;
    else if (string == QLatin1String("right"))
        return Right;
    else if (string == QLatin1String("bottomleft"))
        return BottomLeft;
    else if (string == QLatin1String("bottom"))
        return Bottom;
    else if (string == QLatin1String("bottomright"))
        return BottomRight;

    return Unspecified;
}
