/*
 * customtype.cpp
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

#include "customtype.h"

namespace Tiled {

int CustomType::nextId = 0;

void CustomType::addValue(const QString &name)
{
    values << name;
    validateValues();
}

void CustomType::validateValues()
{
    values.removeAll(QString());
    values.removeDuplicates();
}

QVariant CustomType::wrap(QVariant value) const
{
    return QVariant::fromValue(CustomValue { value, id });
}

QVariant CustomType::defaultValue() const
{
    // todo: should depend on the valueType

    if (!values.isEmpty())
        return values.first();

    return QString();
}

QVariantHash CustomType::toVariant() const
{
    return {
        { QStringLiteral("name"), name },
        { QStringLiteral("values"), values },
        { QStringLiteral("color"), color },
    };
}

CustomType CustomType::fromVariant(const QVariant &variant)
{
    const auto hash = variant.toHash();

    CustomType customType;
    customType.name = hash.value(QStringLiteral("name")).toString();
    customType.values = hash.value(QStringLiteral("values")).toStringList();
    customType.color = hash.value(QStringLiteral("color")).toString();

    return customType;
}

} // namespace Tiled
