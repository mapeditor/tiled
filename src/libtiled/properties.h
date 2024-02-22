/*
 * properties.h
 * Copyright 2010-2021, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertytype.h"

#include <QJsonArray>
#include <QObject>
#include <QUrl>
#include <QVariantMap>

class QDir;

namespace Tiled {

class PropertyType;

class TILEDSHARED_EXPORT PropertyValue
{
    Q_GADGET
    Q_PROPERTY(QVariant value MEMBER value)
    Q_PROPERTY(int typeId MEMBER typeId)
    Q_PROPERTY(QString typeName READ typeName)

public:
    // needed to work around compilation issue with mingw49
    PropertyValue(const QVariant &value = QVariant(), int typeId = 0)
        : value(value)
        , typeId(typeId)
    {}

    QVariant value;
    int typeId;

    const PropertyType *type() const;
    QString typeName() const;

    bool operator==(const PropertyValue &o) const
    { return typeId == o.typeId && value == o.value; }
};

class TILEDSHARED_EXPORT FilePath
{
    Q_GADGET
    Q_PROPERTY(QUrl url MEMBER url)
    Q_PROPERTY(QString localFile READ localFile WRITE setLocalFile)

public:
    QUrl url;

    QString localFile() const { return url.toLocalFile(); }
    void setLocalFile(const QString &filePath)
    { url = QUrl::fromLocalFile(filePath); }

    bool operator==(const FilePath &o) const
    { return url == o.url; }

    static QString toString(const FilePath &path);
    static FilePath fromString(const QString &string);
};

class TILEDSHARED_EXPORT ObjectRef
{
    Q_GADGET
    Q_PROPERTY(int id MEMBER id)

public:
    int id;

    bool operator==(const ObjectRef &o) const
    { return id == o.id; }

    static int toInt(const ObjectRef &ref) { return ref.id; }
    static ObjectRef fromInt(int id) { return ObjectRef { id }; }
};

class TILEDSHARED_EXPORT ExportContext
{
public:
    explicit ExportContext(const QString &path = QString());
    ExportContext(const PropertyTypes &types, const QString &path)
        : mTypes(types)
        , mPath(path)
    {}

    // need to prevent this one since we're only holding a reference to the types
    ExportContext(const PropertyTypes &&types, const QString &path) = delete;

    const PropertyTypes &types() const { return mTypes; }
    const QString &path() const { return mPath; }

    ExportValue toExportValue(const QVariant &value) const;
    QVariant toPropertyValue(const ExportValue &exportValue) const;
    QVariant toPropertyValue(const QVariant &value, int metaType) const;

private:
    const PropertyTypes &mTypes;
    const QString mPath;
};

class TILEDSHARED_EXPORT AggregatedPropertyData
{
public:
    AggregatedPropertyData()
    {}

    explicit AggregatedPropertyData(const QVariant &value)
        : mValue(value)
        , mPresenceCount(1)
    {}

    void aggregate(const QVariant &value)
    {
        mValueConsistent &= value == mValue;
        mPresenceCount += 1;
    }

    const QVariant &value() const { return mValue; }
    int presenceCount() const { return mPresenceCount; }
    bool valueConsistent() const { return mValueConsistent; }

    bool operator==(const AggregatedPropertyData &other) const
    {
        return mValue == other.mValue &&
                mPresenceCount == other.mPresenceCount &&
                mValueConsistent == other.mValueConsistent;
    }

private:
    QVariant mValue;
    int mPresenceCount = 0;
    bool mValueConsistent = true;
};

/**
 * Collection of properties and their values.
 */
using Properties = QVariantMap;


/**
 * Collection of properties with information about the consistency of their
 * presence and value over several property collections.
 */
using AggregatedProperties = QMap<QString, AggregatedPropertyData>;

TILEDSHARED_EXPORT bool setClassPropertyMemberValue(QVariant &classValue,
                                                    int depth,
                                                    const QStringList &path,
                                                    const QVariant &value);

TILEDSHARED_EXPORT bool setPropertyMemberValue(Properties &properties,
                                               const QStringList &path,
                                               const QVariant &value);

TILEDSHARED_EXPORT void aggregateProperties(AggregatedProperties &aggregated, const Properties &properties);
TILEDSHARED_EXPORT void mergeProperties(Properties &target, const Properties &source);

TILEDSHARED_EXPORT QJsonArray propertiesToJson(const Properties &properties, const ExportContext &context = ExportContext());
TILEDSHARED_EXPORT Properties propertiesFromJson(const QJsonArray &json, const ExportContext &context = ExportContext());

TILEDSHARED_EXPORT int propertyValueId();
TILEDSHARED_EXPORT int filePathTypeId();
TILEDSHARED_EXPORT int objectRefTypeId();

TILEDSHARED_EXPORT QString typeToName(int type);
TILEDSHARED_EXPORT QString typeName(const QVariant &value);

TILEDSHARED_EXPORT void initializeMetatypes();

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::FilePath)
Q_DECLARE_METATYPE(Tiled::ObjectRef)
