
/*
 * customproperties.h
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
#include <QStringList>
#include <QColor>
#include <QVector>
#include <QMetaType>

#include "properties.h"

namespace Tiled {

/**
 * Defines a custom property type.
 */
struct CustomType
{
    CustomType()
        : color(Qt::gray)
    {}

    CustomType(QString name, const QColor &color)
        : name(std::move(name))
        , color(color)
    {}

    CustomType(const CustomType &prop)
    {
        name  = prop.name;
        color = prop.color;
        values = prop.values;
        currentIndex = 0;
    }

    ~CustomType()
    {}

    QString name;
    QColor color;
    QStringList values;

    int currentIndex;

    void addValue(const QString &name)
    {
        values << name;
        validateValues();
    }

    void validateValues()
    {
        values.removeAll(QString());
        values.removeDuplicates();
    }

    QString currentValue() const
    {
        if (currentIndex !=-1)
            return values.at(currentIndex);

        return QString::fromUtf8("No value");
    }

    void setValue(const QString &value)
    {
        currentIndex = values.indexOf(value);
    }

    void setIndex(int index)
    {
        if (index < values.size() && index >= -1)
            currentIndex = index;
    }
    
    QVariantHash toVariant() const
    {
        return {
            { QStringLiteral("name"), name },
            { QStringLiteral("values"), values },
            { QStringLiteral("color"), color },
        };
    }

    static CustomType fromVariant(const QVariant &variant)
    {
        const auto hash = variant.toHash();

        CustomType customType;
        customType.name = hash.value(QStringLiteral("name")).toString();
        customType.values = hash.value(QStringLiteral("values")).toStringList();
        customType.color = hash.value(QStringLiteral("color")).toString();

        return customType;
    }

    static QString toString(const CustomType &customType)
    {
        return customType.currentValue();
    }
};

typedef QVector<CustomType> CustomTypes;

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::CustomType);
