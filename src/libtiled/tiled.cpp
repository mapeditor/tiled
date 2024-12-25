/*
 * tiled.cpp
 * Copyright 2017-2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include <QImageReader>

QPointF Tiled::alignmentOffset(const QSizeF &size, Alignment alignment)
{
    switch (alignment) {
    case Unspecified:   break;
    case TopLeft:       break;
    case Top:           return QPointF(size.width() / 2, 0);
    case TopRight:      return QPointF(size.width(), 0);
    case Left:          return QPointF(0, size.height() / 2);
    case Center:        return QPointF(size.width() / 2, size.height() / 2);
    case Right:         return QPointF(size.width(), size.height() / 2);
    case BottomLeft:    return QPointF(0, size.height());
    case Bottom:        return QPointF(size.width() / 2, size.height());
    case BottomRight:   return QPointF(size.width(), size.height());
    }
    return QPointF();
}

Tiled::Alignment Tiled::flipAlignment(Alignment alignment, FlipDirection direction)
{
    switch (direction) {
    case FlipHorizontally:
        switch (alignment) {
        case Unspecified:   Q_ASSERT(false); break;
        case TopLeft:       return TopRight;
        case Top:           return Top;
        case TopRight:      return TopLeft;
        case Left:          return Right;
        case Center:        return Center;
        case Right:         return Left;
        case BottomLeft:    return BottomRight;
        case Bottom:        return Bottom;
        case BottomRight:   return BottomLeft;
        }
        break;
    case FlipVertically:
        switch (alignment) {
        case Unspecified:   Q_ASSERT(false); break;
        case TopLeft:       return BottomLeft;
        case Top:           return Bottom;
        case TopRight:      return BottomRight;
        case Left:          return Left;
        case Center:        return Center;
        case Right:         return Right;
        case BottomLeft:    return TopLeft;
        case Bottom:        return Top;
        case BottomRight:   return TopRight;
        }
        break;
    }
    return alignment;
}

QString Tiled::toFileReference(const QUrl &url, const QString &path)
{
    if (url.isEmpty())
        return QString();

    if (url.isLocalFile()) {
        QString localFile = url.toLocalFile();

        if (path.isEmpty())
            return localFile;

        // Local files can be referred to by relative path
        return QDir(path).relativeFilePath(localFile);
    }

    return url.toString();
}

QUrl Tiled::toUrl(const QString &filePathOrUrl, const QString &path)
{
    if (filePathOrUrl.isEmpty())
        return QUrl();

    // What appears to be a relative path may be a non-relative URL
    if (QDir::isRelativePath(filePathOrUrl)) {
        const QUrl url(filePathOrUrl, QUrl::StrictMode);
        if (!url.isRelative())
            return url;
    }

    QString filePath = filePathOrUrl;

    // Resolve possible relative file reference
    if (!path.isEmpty())
        filePath = QDir::cleanPath(QDir(path).filePath(filePath));

    if (filePath.startsWith(QLatin1String(":/")))
        return QUrl(QLatin1String("qrc") + filePath);

    return QUrl::fromLocalFile(filePath);
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

QString Tiled::filePathRelativeTo(const QDir &dir, const QString &filePath)
{
    // We can't refer to files loaded from extensions or resources using relative paths
    if (filePath.startsWith(QLatin1String("ext:")) ||
            filePath.startsWith(QLatin1String(":"))) {
        return filePath;
    }

    return dir.relativeFilePath(filePath);
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

Tiled::CompatibilityVersion Tiled::versionFromString(const QString &string)
{
    if (string == QLatin1String("1.8"))
        return Tiled_1_8;
    else if (string == QLatin1String("1.9"))
        return Tiled_1_9;
    else if (string == QLatin1String("1.10"))
        return Tiled_1_10;
    else if (string == QLatin1String("latest"))
        return Tiled_Latest;
    return UnknownVersion;
}

void Tiled::increaseImageAllocationLimit(int mbLimit)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Adjust the allocation limit to accommodate larger images
    const int currentLimit = QImageReader::allocationLimit();
    if (currentLimit && currentLimit < mbLimit)
        QImageReader::setAllocationLimit(mbLimit);
#else
    Q_UNUSED(mbLimit);
#endif
}
