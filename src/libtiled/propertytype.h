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
#include <QMetaType>

#include "tiled_global.h"

#include <memory>
#include <vector>

namespace Tiled {

class ExportContext;

/**
 * Defines a custom property type. Currently this includes only enums.
 */
class TILEDSHARED_EXPORT PropertyType
{
public:
    enum Type {
        PT_Invalid,
        PT_Class,
        PT_Enum
    };

    const Type type;
    int id = 0;
    QString name;

    virtual ~PropertyType() = default;

    virtual QVariant wrap(const QVariant &value) const;
    virtual QVariant unwrap(const QVariant &value) const;

    virtual QVariant defaultValue() const = 0;

    virtual QVariantHash toVariant(const ExportContext &) const;
    virtual void fromVariant(const QVariantHash &variant, const ExportContext &) = 0;

    static int nextId;

    static std::unique_ptr<PropertyType> createFromVariant(const QVariant &variant,
                                                           const ExportContext &context);

    static Type typeFromString(const QString &string);
    static QString typeToString(Type type);

protected:
    PropertyType(Type type, const QString &name)
        : type(type)
        , name(name)
    {}
};

class TILEDSHARED_EXPORT EnumPropertyType : public PropertyType
{
public:
    enum StorageType {
        IntValue,
        StringValue
    };

    StorageType storageType = StringValue;   // TODO: Allow user to change this
    QStringList values;

    EnumPropertyType(const QString &name) : PropertyType(PT_Enum, name) {}

    QVariant wrap(const QVariant &value) const override;
    QVariant unwrap(const QVariant &value) const override;

    QVariant defaultValue() const override;

    QVariantHash toVariant(const ExportContext &) const override;
    void fromVariant(const QVariantHash &variant, const ExportContext &) override;

    static StorageType storageTypeFromString(const QString &string);
    static QString storageTypeToString(StorageType type);
};

class TILEDSHARED_EXPORT ClassPropertyType : public PropertyType
{
public:
    QVariantMap members;

    ClassPropertyType(const QString &name) : PropertyType(PT_Class, name) {}

    QVariant defaultValue() const override;

    QVariantHash toVariant(const ExportContext &context) const override;
    void fromVariant(const QVariantHash &variant, const ExportContext & ) override;
};

using PropertyTypes = std::vector<std::unique_ptr<PropertyType>>;

TILEDSHARED_EXPORT const PropertyType *findTypeById(const PropertyTypes &types, int typeId);
TILEDSHARED_EXPORT const PropertyType *findTypeByName(const PropertyTypes &types, const QString &name);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::PropertyType*);
