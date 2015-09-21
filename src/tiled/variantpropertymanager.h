/*
 * variantpropertymanager.h
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VARIANTPROPERTYMANAGER_H
#define VARIANTPROPERTYMANAGER_H

#include <QtVariantPropertyManager>

namespace Tiled {
namespace Internal {

/**
 * Extension of the QtVariantPropertyManager that adds support for a filePath
 * data type.
 */
class VariantPropertyManager : public QtVariantPropertyManager
{
    Q_OBJECT

public:
    explicit VariantPropertyManager(QObject *parent = nullptr)
        : QtVariantPropertyManager(parent)
        , mSuggestionsAttribute(QLatin1String("suggestions"))
    {}

    QVariant value(const QtProperty *property) const override;
    int valueType(int propertyType) const override;
    bool isPropertyTypeSupported(int propertyType) const override;

    QStringList attributes(int propertyType) const override;
    int attributeType(int propertyType, const QString &attribute) const override;
    QVariant attributeValue(const QtProperty *property,
                            const QString &attribute) const override;

    static int filePathTypeId();

public slots:
    void setValue(QtProperty *property, const QVariant &val) override;
    void setAttribute(QtProperty *property,
                      const QString &attribute,
                      const QVariant &value) override;

protected:
    QString valueText(const QtProperty *property) const override;
    void initializeProperty(QtProperty *property) override;
    void uninitializeProperty(QtProperty *property) override;

private:
    struct Data {
        QString value;
        QString filter;
    };
    QMap<const QtProperty *, Data> mValues;
    QMap<const QtProperty *, QStringList> mSuggestions;

    const QString mSuggestionsAttribute;
};

} // namespace Internal
} // namespace Tiled

#endif // VARIANTPROPERTYMANAGER_H
