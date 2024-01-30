/*
 * scriptimage.cpp
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptimage.h"

#include "scriptmanager.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QJSEngine>

namespace Tiled {

ScriptImage::ScriptImage(QObject *parent)
    : QObject(parent)
{}

ScriptImage::ScriptImage(int width, int height, Format format, QObject *parent)
    : QObject(parent)
    , mImage(width, height, static_cast<QImage::Format>(format))
{}

ScriptImage::ScriptImage(const QByteArray &data, int width, int height, Format format, QObject *parent)
    : QObject(parent)
    , mImageData(data)
    , mImage(reinterpret_cast<const uchar*>(mImageData.constData()), width, height, static_cast<QImage::Format>(format))
{}

ScriptImage::ScriptImage(const QByteArray &data, int width, int height, int bytesPerLine, Format format, QObject *parent)
    : QObject(parent)
    , mImageData(data)
    , mImage(reinterpret_cast<const uchar*>(mImageData.constData()), width, height, bytesPerLine, static_cast<QImage::Format>(format))
{}

ScriptImage::ScriptImage(const QString &fileName, const QByteArray &format, QObject *parent)
    : QObject(parent)
    , mImage(fileName, format.isEmpty() ? nullptr : format.data())
{}

QByteArray ScriptImage::saveToData(const QByteArray &format, int quality)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    if (mImage.save(&buffer, format, quality))
        return ba;
    return QByteArray();
}

QJSValue ScriptImage::colorTable() const
{
    QJSEngine *engine = qjsEngine(this);
    if (!engine)
        return QJSValue();

    const auto colors = mImage.colorTable();

    QJSValue array = engine->newArray(colors.size());
    for (int i = 0; i < colors.size(); ++i)
        array.setProperty(i, colors.at(i));

    return array;
}

void ScriptImage::setColorTable(QJSValue colors)
{
    QVector<QRgb> colorTable;

    const int length = colors.property(QStringLiteral("length")).toInt();
    colorTable.resize(length);

    for (int i = 0; i < length; ++i) {
        const QJSValue color = colors.property(i);
        if (color.isNumber()) {
            colorTable[i] = color.toUInt();
        } else if (color.isString()) {
            const QString colorName = color.toString();
            if (QColor::isValidColor(colorName)) {
                colorTable[i] = QColor(colorName).rgba();
            } else {
                ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                                 "Invalid color name: '%2'").arg(colorName));
                return;
            }
        } else {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid color value"));
            return;
        }
    }

    mImage.setColorTable(std::move(colorTable));
}

ScriptImage *ScriptImage::copy(int x, int y, int w, int h) const
{
    return new ScriptImage(mImage.copy(x, y, w, h));
}

ScriptImage *ScriptImage::scaled(int w, int h,
                                 AspectRatioMode aspectMode,
                                 TransformationMode mode) const
{
    return new ScriptImage(mImage.scaled(w, h,
                                         static_cast<Qt::AspectRatioMode>(aspectMode),
                                         static_cast<Qt::TransformationMode>(mode)));
}

ScriptImage *ScriptImage::mirrored(bool horiz, bool vert) const
{
    return new ScriptImage(mImage.mirrored(horiz, vert));
}

} // namespace Tiled

#include "moc_scriptimage.cpp"
