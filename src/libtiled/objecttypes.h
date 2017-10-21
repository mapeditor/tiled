/*
 * objecttypes.h
 * Copyright 2011-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QString>
#include <QColor>
#include <QVector>

#include "properties.h"

namespace Tiled {

/**
 * Quick definition of an object type. It has a name and a color.
 */
struct ObjectType
{
    ObjectType() : color(Qt::gray) {}

    ObjectType(QString name,
               const QColor &color,
               const Properties &properties = Properties())
        : name(std::move(name))
        , color(color)
        , defaultProperties(properties)
    {}

    QString name;
    QColor color;

    Properties defaultProperties;
};

typedef QVector<ObjectType> ObjectTypes;


class TILEDSHARED_EXPORT ObjectTypesSerializer
{
public:
    enum Format {
        Autodetect,
        Xml,
        Json
    };

    ObjectTypesSerializer(Format format = Autodetect);

    bool writeObjectTypes(const QString &fileName,
                          const ObjectTypes &objectTypes);

    bool readObjectTypes(const QString &fileName,
                         ObjectTypes &objectTypes);

    QString errorString() const { return mError; }

private:
    Format mFormat;
    QString mError;
};

} // namespace Tiled
