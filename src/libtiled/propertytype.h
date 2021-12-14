/*
 * propertytype.h
 * Copyright 2021, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QStringList>
#include <QVariant>
#include <QVector>
#include <QMetaType>

#include "tiled_global.h"

namespace Tiled {

/**
 * Defines a custom property type. Currently this includes only enums.
 */
class TILEDSHARED_EXPORT PropertyType
{
public:
    int id = 0;
    QString name;
    QStringList values;

    static int nextId;

    QVariant wrap(const QVariant &value) const;
    QVariant unwrap(const QVariant &value) const;

    QVariant defaultValue() const;

    QVariantHash toVariant() const;
    static PropertyType fromVariant(const QVariant &variant);
};

using PropertyTypes = QVector<PropertyType>;

TILEDSHARED_EXPORT const PropertyType *findTypeById(const QVector<PropertyType> &types, int typeId);
TILEDSHARED_EXPORT const PropertyType *findTypeByName(const QVector<PropertyType> &types, const QString &name);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::PropertyType);
