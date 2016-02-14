/*
 * properties.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "properties.h"

namespace Tiled {

void Properties::merge(const Properties &other)
{
    // Based on QMap::unite, but using insert instead of insertMulti
    const_iterator it = other.constEnd();
    const const_iterator b = other.constBegin();
    while (it != b) {
        --it;
        insert(it.key(), it.value());
    }
}

void AggregatedProperties::aggregate(const Properties &properties)
{
    auto it = properties.constEnd();
    const auto b = properties.constBegin();
    while (it != b) {
        --it;

        auto pit = find(it.key());
        if (pit != end()) {
            AggregatedPropertyData &propertyData = pit.value();
            propertyData.aggregate(it.value());
        } else {
            insert(it.key(), AggregatedPropertyData(it.value()));
        }
    }

    ++mAggregatedCount;
}

QString typeToName(QVariant::Type type)
{
    if (type == QVariant::String)
        return QStringLiteral("string");
    if (type == QVariant::Double)
        return QStringLiteral("float");

    return QLatin1String(QVariant::typeToName(type));
}

QVariant::Type nameToType(const QString &name)
{
    if (name == QLatin1String("string"))
        return QVariant::String;
    if (name == QLatin1String("float"))
        return QVariant::Double;

    return QVariant::nameToType(name.toLatin1().constData());
}

} // namespace Tiled
