/*
 * properties.h
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

#pragma once

#include "tiled_global.h"

#include <QJsonArray>
#include <QObject>
#include <QUrl>
#include <QVariantMap>

class QDir;

namespace Tiled {

class TILEDSHARED_EXPORT FilePath
{
    Q_GADGET
    Q_PROPERTY(QUrl url MEMBER url)

public:
    QUrl url;

    static QString toString(const FilePath &path);
    static FilePath fromString(const QString &string);
};

struct TILEDSHARED_EXPORT ObjectRef
{
    Q_GADGET
    Q_PROPERTY(int id MEMBER id)

public:
    int id;

    static int toInt(const ObjectRef &ref) { return ref.id; }
    static ObjectRef fromInt(int id) { return ObjectRef { id }; }
};

class TILEDSHARED_EXPORT AggregatedPropertyData
{
public:
    AggregatedPropertyData()
        : mPresenceCount(0)
        , mValueConsistent(true)
    {}

    explicit AggregatedPropertyData(const QVariant &value)
        : mValue(value)
        , mPresenceCount(1)
        , mValueConsistent(true)
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
    int mPresenceCount;
    bool mValueConsistent;
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

TILEDSHARED_EXPORT void aggregateProperties(AggregatedProperties &aggregated, const Properties &properties);
TILEDSHARED_EXPORT void mergeProperties(Properties &target, const Properties &source);

TILEDSHARED_EXPORT QJsonArray propertiesToJson(const Properties &properties);
TILEDSHARED_EXPORT Properties propertiesFromJson(const QJsonArray &json);

TILEDSHARED_EXPORT int filePathTypeId();
TILEDSHARED_EXPORT int objectRefTypeId();

TILEDSHARED_EXPORT QString typeToName(int type);
TILEDSHARED_EXPORT int nameToType(const QString &name);

TILEDSHARED_EXPORT QVariant toExportValue(const QVariant &value);
TILEDSHARED_EXPORT QVariant fromExportValue(const QVariant &value, int type);

TILEDSHARED_EXPORT QVariant toExportValue(const QVariant &value, const QDir &dir);
TILEDSHARED_EXPORT QVariant fromExportValue(const QVariant &value, int type, const QDir &dir);

TILEDSHARED_EXPORT void initializeMetatypes();

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::FilePath)
Q_DECLARE_METATYPE(Tiled::ObjectRef)
