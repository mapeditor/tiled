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

#include "properties.h"
#include "propertiesview.h"
#include "propertytype.h"

#include <QSet>

namespace Tiled {

class Document;
class MapDocument;

/**
 * An interface used by ObjectRefProperty to access the current MapDocument.
 */
class MapDocumentContext
{
public:
    virtual ~MapDocumentContext() = default;
    virtual MapDocument *mapDocument() const = 0;
};

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
    void setMemberValue(const PropertyPath &path, const QVariant &value);

    Property *property(const QString &name) const;

signals:
    void memberValueChanged(const PropertyPath &path, const QVariant &value);

protected:
    virtual void propertyTypesChanged();

    Document *mDocument = nullptr;
    bool mPropertyTypesChanged = false;

private:
    bool createOrUpdateProperty(int index,
                                const QString &name,
                                const QVariant &oldValue,
                                const QVariant &newValue);

    void emitMemberValueChanged(const PropertyPath &path, const QVariant &value);

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
 * A property that creates child properties based on a ClassPropertyType.
 */
class ClassProperty : public GroupProperty
{
    Q_OBJECT

public:
    ClassProperty(const QString &name,
                  const PropertyPath &path,
                  const ClassPropertyType &classType,
                  std::function<QVariantMap ()> get,
                  std::function<void(const PropertyPath &path, const QVariant &value)> set,
                  QObject *parent = nullptr);

    DisplayMode displayMode() const override { return DisplayMode::Default; }

    QVariantMap value() const { return mGet(); }

signals:
    void memberValueChanged(const PropertyPath &path, const QVariant &value);

private:
    void createMembers(const ClassPropertyType &classType);

    bool createOrUpdateProperty(int index,
                                const QString &name,
                                const QVariant &oldValue,
                                const QVariant &newValue);

    PropertyPath mPath;
    QHash<QString, Property*> mPropertyMap;
    std::function<QVariantMap ()> mGet;
    std::function<void(const PropertyPath &path, const QVariant &value)> mSet;
    bool mEmittingValueChanged = false;
};


/**
 * A property that creates child properties based on a QVariantList value.
 */
class VariantListProperty : public GroupProperty
{
    Q_OBJECT

public:
    VariantListProperty(const QString &name,
                        const PropertyPath &path,
                        std::function<QVariantList()> get,
                        std::function<void(const PropertyPath &, const QVariant &)> set,
                        QObject *parent = nullptr);

    void setValue(const QVariantList &value);
    QVariantList value() const { return mGet(); }
    QVariantList valuesAt(const QList<int> &indices) const;

    void removeValueAt(int index);
    void removeValuesAt(const QList<int> &indices);
    void addValue(const QVariant &value);
    void insertValuesAt(int index, const QVariantList &values);

    QWidget *createEditor(QWidget *parent) override;
    void addContextMenuActions(QMenu *menu) override;

private:
    bool createOrUpdateProperty(int index,
                                const QVariant &oldValue,
                                const QVariant &newValue);

    PropertyPath mPath;
    std::function<QVariantList ()> mGet;
    std::function<void(const PropertyPath &path, const QVariant &value)> mSet;
    QVariantList mOldValues;    // remembered solely so we know the types we created properties for
    bool mEmittingValueChanged = false;
};


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
    QString m_placeholderText;
    QVariant m_value;
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
