/*
 * tmxmapformat.h
 * Copyright 2008-2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "mapformat.h"
#include "objecttemplateformat.h"
#include "tiled_global.h"
#include "tilesetformat.h"

namespace Tiled {

class Tileset;
class MapObject;

/**
 * A reader and writer for Tiled's .tmx map format.
 */
class TILEDSHARED_EXPORT TmxMapFormat : public MapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    TmxMapFormat(QObject *parent = nullptr);

    std::unique_ptr<Map> read(const QString &fileName) override;

    bool write(const Map *map, const QString &fileName, Options options) override;

    /**
     * Converts the given map to a utf8 byte array (in .tmx format). This is
     * for storing a map in the clipboard. References to other files (like
     * tileset images) will be saved as absolute paths.
     *
     * @see fromByteArray
     */
    QByteArray toByteArray(const Map *map);

    /**
     * Reads the map given from \a data. This is for retrieving a map from the
     * clipboard. Returns null on failure.
     *
     * @see toByteArray
     */
    std::unique_ptr<Map> fromByteArray(const QByteArray &data);

    QString nameFilter() const override { return tr("Tiled map files (*.tmx *.xml)"); }

    QString shortName() const override { return QStringLiteral("tmx"); }

    bool supportsFile(const QString &fileName) const override;

    QString errorString() const override { return mError; }

private:
    QString mError;
};

/**
 * A reader and writer for Tiled's .tsx tileset format.
 */
class TILEDSHARED_EXPORT TsxTilesetFormat : public TilesetFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::TilesetFormat)

public:
    TsxTilesetFormat(QObject *parent = nullptr);

    SharedTileset read(const QString &fileName) override;

    bool write(const Tileset &tileset, const QString &fileName, Options options) override;

    QString nameFilter() const override { return tr("Tiled tileset files (*.tsx *.xml)"); }

    QString shortName() const override { return QStringLiteral("tsx"); }

    bool supportsFile(const QString &fileName) const override;

    QString errorString() const override { return mError; }

private:
    QString mError;
};

/**
 * A reader and writer for Tiled's .tx template format.
 */
class TILEDSHARED_EXPORT XmlObjectTemplateFormat : public ObjectTemplateFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::ObjectTemplateFormat)

public:
    XmlObjectTemplateFormat(QObject *parent = nullptr);

    std::unique_ptr<ObjectTemplate> read(const QString &fileName) override;

    bool write(const ObjectTemplate *objectTemplate, const QString &fileName) override;

    QString nameFilter() const override { return tr("Tiled template files (*.tx)"); }

    QString shortName() const override { return QStringLiteral("tx"); }

    bool supportsFile(const QString &fileName) const override;

    QString errorString() const override { return mError; }

private:
    QString mError;
};

} // namespace Tiled
