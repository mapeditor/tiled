/*
 * editableobject.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "object.h"

#include <QObject>

namespace Tiled {

class Document;
class EditableAsset;

/**
 * Editable wrapper class, enabling objects to be inspected and modified from
 * scripts.
 *
 * Generally, editables are created on-demand and are owned by the script
 * (garbage collected). This excludes EditableAsset instances, which are owned
 * by their Document.
 */
class EditableObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Tiled::EditableAsset *asset READ asset)
    Q_PROPERTY(bool readOnly READ isReadOnly)
    Q_PROPERTY(QString className READ className WRITE setClassName)

public:
    EditableObject(EditableAsset *asset,
                   Object *object,
                   QObject *parent = nullptr);

    EditableAsset *asset() const;

    virtual bool isReadOnly() const;
    bool checkReadOnly() const;

    const QString &className() const;

    Q_INVOKABLE QVariant property(const QString &name) const;
    Q_INVOKABLE void setProperty(const QString &name, const QVariant &value);
    Q_INVOKABLE void setColorProperty(const QString &name, const QColor &value);
    Q_INVOKABLE void setColorProperty(const QString &name, int r, int g, int b, int a = 255);
    Q_INVOKABLE void setFloatProperty(const QString &name, qreal value);

    Q_INVOKABLE QVariantMap properties() const;
    Q_INVOKABLE void setProperties(const QVariantMap &properties);

    Q_INVOKABLE void removeProperty(const QString &name);

    Q_INVOKABLE QVariant resolvedProperty(const QString &name) const;
    Q_INVOKABLE QVariantMap resolvedProperties() const;

    Object *object() const;
    Document *document() const;

    void setAsset(EditableAsset *asset);
    void setObject(Object *object);

    static EditableObject *find(Object *object);

public slots:
    void setClassName(const QString &type);

protected:
    bool moveOwnershipToJavaScript();
    void moveOwnershipToCpp();

private:
    QVariant toScript(const QVariant &value) const;
    QVariant fromScript(const QVariant &value) const;
    QVariantMap toScript(const QVariantMap &value) const;
    QVariantMap fromScript(const QVariantMap &value) const;

    EditableAsset *mAsset;
    Object *mObject;
};


inline EditableAsset *EditableObject::asset() const
{
    return mAsset;
}

inline const QString &EditableObject::className() const
{
    return object()->className();
}

inline QVariant EditableObject::property(const QString &name) const
{
    return toScript(mObject->property(name));
}

inline void EditableObject::setColorProperty(const QString &name, const QColor &value)
{
    setProperty(name, value);
}

inline void EditableObject::setColorProperty(const QString &name, int r, int g, int b, int a)
{
    setProperty(name, QColor(r, g, b, a));
}

inline void EditableObject::setFloatProperty(const QString &name, qreal value)
{
    setProperty(name, value);
}

inline QVariantMap EditableObject::properties() const
{
    return toScript(mObject->properties());
}

inline QVariant EditableObject::resolvedProperty(const QString &name) const
{
    return toScript(mObject->resolvedProperty(name));
}

inline QVariantMap EditableObject::resolvedProperties() const
{
    return toScript(mObject->resolvedProperties());
}

inline Object *EditableObject::object() const
{
    return mObject;
}

inline void EditableObject::setAsset(EditableAsset *asset)
{
    mAsset = asset;
}

inline EditableObject *EditableObject::find(Object *object)
{
    return object ? static_cast<EditableObject*>(object->mEditable.data())
                  : nullptr;
}

} // namespace Tiled
