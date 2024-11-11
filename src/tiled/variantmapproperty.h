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

#include "propertytype.h"
#include "varianteditor.h"

namespace Tiled {

class Document;

class VariantMapProperty : public GroupProperty
{
    Q_OBJECT

public:
    VariantMapProperty(const QString &name, QObject *parent = nullptr);

    void setValue(const QVariantMap &value,
                  const QVariantMap &suggestions = {});

    const QVariantMap &value() const { return mValue; }

    Property *property(const QString &name) const;

signals:
    void memberValueChanged(const QStringList &path, const QVariant &value);
    void renameRequested(const QString &name);

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

    void removeMember(const QString &name);
    void addMember(const QString &name, const QVariant &value);
    void setClassMember(const QStringList &path, const QVariant &value);

    void updateModifiedRecursively(Property *property, const QVariant &value);
    void emitValueChangedRecursively(Property *property);

    void emitMemberValueChanged(const QStringList &path, const QVariant &value);

    void memberContextMenuRequested(Property *property, const QStringList &path, const QPoint &globalPos);

    QIcon m_resetIcon;
    QIcon m_removeIcon;
    QIcon m_addIcon;
    QIcon m_renameIcon;
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

} // namespace Tiled
