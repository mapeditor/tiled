/*
 * scriptimage.h
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

#pragma once

#include <QImage>
#include <QJSValue>
#include <QObject>

namespace Tiled {

class ScriptImage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(int depth READ depth)
    Q_PROPERTY(QSize size READ size)
    Q_PROPERTY(Format format READ format)

public:
    // Copied from QImage
    enum Format {
        Format_Invalid,
        Format_Mono,
        Format_MonoLSB,
        Format_Indexed8,
        Format_RGB32,
        Format_ARGB32,
        Format_ARGB32_Premultiplied,
        Format_RGB16,
        Format_ARGB8565_Premultiplied,
        Format_RGB666,
        Format_ARGB6666_Premultiplied,
        Format_RGB555,
        Format_ARGB8555_Premultiplied,
        Format_RGB888,
        Format_RGB444,
        Format_ARGB4444_Premultiplied,
        Format_RGBX8888,
        Format_RGBA8888,
        Format_RGBA8888_Premultiplied,
        Format_BGR30,
        Format_A2BGR30_Premultiplied,
        Format_RGB30,
        Format_A2RGB30_Premultiplied,
        Format_Alpha8,
        Format_Grayscale8,
        Format_RGBX64,
        Format_RGBA64,
        Format_RGBA64_Premultiplied,
        Format_Grayscale16,
        Format_BGR888,
#ifndef Q_QDOC
        NImageFormats
#endif
    };
    Q_ENUM(Format)

    enum AspectRatioMode {
        IgnoreAspectRatio           = Qt::IgnoreAspectRatio,
        KeepAspectRatio             = Qt::KeepAspectRatio,
        KeepAspectRatioByExpanding  = Qt::KeepAspectRatioByExpanding
    };
    Q_ENUM(AspectRatioMode)

    enum TransformationMode {
        FastTransformation          = Qt::FastTransformation,
        SmoothTransformation        = Qt::SmoothTransformation
    };
    Q_ENUM(TransformationMode)

    Q_INVOKABLE explicit ScriptImage(QObject *parent = nullptr);
    Q_INVOKABLE ScriptImage(int width, int height, Format format = Format_ARGB32_Premultiplied, QObject *parent = nullptr);
    Q_INVOKABLE ScriptImage(const QByteArray &data, int width, int height, Format format, QObject *parent = nullptr);
    Q_INVOKABLE ScriptImage(const QByteArray &data, int width, int height, int bytesPerLine, Format format, QObject *parent = nullptr);
    Q_INVOKABLE ScriptImage(const QString &fileName, const QByteArray &format = QByteArray(), QObject *parent = nullptr);

    ScriptImage(QImage image)
        : mImage(std::move(image))
    {}

    Format format() const { return static_cast<Format>(mImage.format()); }
    int width() const { return mImage.width(); }
    int height() const { return mImage.height(); }
    int depth() const { return mImage.depth(); }
    QSize size() const { return mImage.size(); }

    Q_INVOKABLE uint pixel(int x, int y) const
    { return mImage.pixel(x, y); }

    Q_INVOKABLE QColor pixelColor(int x, int y) const
    { return mImage.pixelColor(x, y); }

    Q_INVOKABLE void setPixel(int x, int y, uint index_or_rgb)
    { mImage.setPixel(x, y, index_or_rgb); }

    Q_INVOKABLE void setPixelColor(int x, int y, const QColor &color)
    { mImage.setPixelColor(x, y, color); }

    Q_INVOKABLE void fill(uint index_or_rgb)
    { mImage.fill(index_or_rgb); }

    Q_INVOKABLE void fill(const QColor &color)
    { mImage.fill(color); }

    Q_INVOKABLE bool load(const QString &fileName, const QByteArray &format = QByteArray())
    { return mImage.load(fileName, format); }

    Q_INVOKABLE bool loadFromData(const QByteArray &data, const QByteArray &format = QByteArray())
    { return mImage.loadFromData(data, format); }

    Q_INVOKABLE bool save(const QString &fileName, const QByteArray &format = QByteArray(), int quality = -1)
    { return mImage.save(fileName, format, quality); }

    Q_INVOKABLE QByteArray saveToData(const QByteArray &format = "PNG", int quality = -1);

    Q_INVOKABLE uint color(int i) const
    { return mImage.color(i); }

    Q_INVOKABLE QJSValue colorTable() const;

    Q_INVOKABLE void setColor(int i, uint rgb)
    { mImage.setColor(i, rgb); }

    Q_INVOKABLE void setColor(int i, const QColor &color)
    { mImage.setColor(i, color.rgba()); }

    Q_INVOKABLE void setColorTable(QJSValue colors);

    Q_INVOKABLE Tiled::ScriptImage *copy(int x, int y, int w, int h) const;
    Q_INVOKABLE Tiled::ScriptImage *scaled(int w, int h,
                                           AspectRatioMode aspectMode = IgnoreAspectRatio,
                                           TransformationMode mode = FastTransformation) const;
    Q_INVOKABLE Tiled::ScriptImage *mirrored(bool horiz, bool vert) const;

    const QImage &image() const { return mImage; }

private:
    QByteArray mImageData;
    QImage mImage;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptImage*)
