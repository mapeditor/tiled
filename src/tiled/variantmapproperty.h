/*
 * variantmapproperty.h
 * Copyright 2024, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include "propertiesview.h"
#include "propertytype.h"

#include <QSet>

namespace Tiled {

class Document;

/**
 * A property that creates child properties based on a QVariantMap value.
 */
class VariantMapProperty : public GroupProperty
{
    Q_OBJECT

public:
    VariantMapProperty(const QString &name, QObject *parent = nullptr);

    void setValue(const QVariantMap &value,
                  const QVariantMap &suggestions = {});

    const QVariantMap &value() const { return mValue; }

    void removeMember(const QString &name);
    void addMember(const QString &name, const QVariant &value);
    void setMemberValue(const QStringList &path, const QVariant &value);

    Property *property(const QString &name) const;

signals:
    void memberValueChanged(const QStringList &path, const QVariant &value);

protected:
    virtual void propertyTypesChanged();

    Document *mDocument = nullptr;
    bool mPropertyTypesChanged = false;

private:
    bool createOrUpdateProperty(int index,
                                const QString &name,
                                const QVariant &oldValue,
                                const QVariant &newValue);
    Property *createProperty(const QStringList &path,
                             std::function<QVariant ()> get,
                             std::function<void (const QVariant &)> set);

    void createClassMembers(const QStringList &path,
                            GroupProperty *groupProperty,
                            const ClassPropertyType &classType,
                            std::function<QVariant ()> get);

    void updateModifiedRecursively(Property *property, const QVariant &value);
    void emitValueChangedRecursively(Property *property);

    void emitMemberValueChanged(const QStringList &path, const QVariant &value);

    bool mEmittingValueChanged = false;
    QVariantMap mValue;
    QVariantMap mSuggestions;
    QHash<QString, Property*> mPropertyMap;
    QSet<QStringList> mExpandedProperties;
};

inline Property *VariantMapProperty::property(const QString &name) const
{
    return mPropertyMap.value(name);
}


/**
 * A property that creates widgets for adding a value with a certain name and
 * type.
 */
class AddValueProperty : public Property
{
    Q_OBJECT

public:
    AddValueProperty(QObject *parent = nullptr);

    void setPlaceholderText(const QString &text);
    void setParentClassType(const ClassPropertyType *parentClassType);

    QVariant value() const;

    QWidget *createLabel(int level, QWidget *parent) override;
    QWidget *createEditor(QWidget *parent) override;

signals:
    void placeholderTextChanged(const QString &text);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QIcon m_plainTypeIcon;
    QString m_placeholderText;
    QVariant m_value;
    bool m_hasFocus = false;
    const ClassPropertyType *m_parentClassType = nullptr;
};

inline void AddValueProperty::setParentClassType(const ClassPropertyType *parentClassType)
{
    m_parentClassType = parentClassType;
}

inline QVariant AddValueProperty::value() const
{
    return m_value;
}

} // namespace Tiled
