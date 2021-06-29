/*
 * propertytype.cpp
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

#include "propertytype.h"

namespace Tiled {

int PropertyType::nextId = 0;

QVariant PropertyType::wrap(const QVariant &value) const
{
    // Convert enum values stored as string, if possible
    if (value.userType() == QMetaType::QString) {
        const int index = values.indexOf(value.toString());
        if (index != -1)
            return QVariant::fromValue(PropertyValue { index, id });
    }

    return QVariant::fromValue(PropertyValue { value, id });
}

QVariant PropertyType::unwrap(const QVariant &value) const
{
    // Convert enum values to their string (todo: should be optional)
    if (value.userType() == QMetaType::Int) {
        const int index = value.toInt();
        if (index >= 0 && index < values.size())
            return values.at(index);
    }

    return value;
}

QVariant PropertyType::defaultValue() const
{
    // todo: should depend on the valueType
    return 0;
}

QVariantHash PropertyType::toVariant() const
{
    return {
        { QStringLiteral("id"), id },
        { QStringLiteral("name"), name },
        { QStringLiteral("values"), values },
    };
}

PropertyType PropertyType::fromVariant(const QVariant &variant)
{
    const auto hash = variant.toHash();

    PropertyType propertyType;
    propertyType.id = hash.value(QStringLiteral("id")).toInt();
    propertyType.name = hash.value(QStringLiteral("name")).toString();
    propertyType.values = hash.value(QStringLiteral("values")).toStringList();

    nextId = std::max(nextId, propertyType.id);

    return propertyType;
}

} // namespace Tiled
