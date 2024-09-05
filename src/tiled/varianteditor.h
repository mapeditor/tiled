/*
 * varianteditor.h
 * Copyright 2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QCoreApplication>
#include <QMetaProperty>
#include <QScrollArea>
#include <QString>
#include <QStringListModel>
#include <QVariant>
#include <QWidget>

#include <memory>
#include <unordered_map>

class QGridLayout;

namespace Tiled {

/**
 * A property represents a named value that can create its own edit widget.
 */
class Property : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    Property(const QString &name, QObject *parent = nullptr)
        : QObject(parent)
        , m_name(name)
    {}

    const QString &name() const { return m_name; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled)
    {
        if (m_enabled != enabled) {
            m_enabled = enabled;
            emit enabledChanged(enabled);
        }
    }

    virtual QVariant value() const = 0;
    virtual void setValue(const QVariant &value) = 0;

    virtual QWidget *createEditor(QWidget *parent) = 0;

signals:
    void valueChanged();
    void enabledChanged(bool enabled);

private:
    QString m_name;
    bool m_enabled = true;
};

/**
 * An editor factory is responsible for creating an editor widget for a given
 * property. It can be used to share the configuration of editor widgets
 * between different properties.
 */
class EditorFactory
{
    Q_DECLARE_TR_FUNCTIONS(EditorFactory)

public:
    virtual QWidget *createEditor(Property *property, QWidget *parent) = 0;
};

struct IntEditorFactory : EditorFactory
{
    QWidget *createEditor(Property *property, QWidget *parent) override;
};

struct FloatEditorFactory : EditorFactory
{
    QWidget *createEditor(Property *property, QWidget *parent) override;

    void setSuffix(const QString &suffix) { m_suffix = suffix; }

private:
    QString m_suffix;
};

/**
 * An editor factory that creates a combo box for enum properties.
 */
class EnumEditorFactory : public EditorFactory
{
public:
    EnumEditorFactory(const QStringList &enumNames = {},
                      const QList<int> &enumValues = {});

    void setEnumNames(const QStringList &enumNames);
    void setEnumValues(const QList<int> &enumValues);

    QWidget *createEditor(Property *property, QWidget *parent) override;

private:
    QStringListModel m_enumNamesModel;
    QList<int> m_enumValues;
};

/**
 * A property that uses an editor factory to create its editor, but does not
 * store a value itself.
 *
 * The property does not take ownership of the editor factory.
 */
class AbstractProperty : public Property
{
    Q_OBJECT

public:
    AbstractProperty(const QString &name,
                     EditorFactory *editorFactory,
                     QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent) override;

private:
    EditorFactory *m_editorFactory;
};

/**
 * A property that uses the given functions to get and set the value and uses
 * an editor factory to create its editor.
 *
 * The property does not take ownership of the editor factory.
 */
class GetSetProperty : public AbstractProperty
{
    Q_OBJECT

public:
    GetSetProperty(const QString &name,
                   std::function<QVariant()> get,
                   std::function<void(const QVariant&)> set,
                   EditorFactory *editorFactory,
                   QObject *parent = nullptr);

    QVariant value() const override { return m_get(); }
    void setValue(const QVariant &value) override { m_set(value); }

private:
    std::function<QVariant()> m_get;
    std::function<void(const QVariant&)> m_set;
};

/**
 * A property that stores a value of a given type and uses an editor factory to
 * create its editor.
 *
 * The property does not take ownership of the editor factory.
 */
class ValueProperty : public AbstractProperty
{
    Q_OBJECT

public:
    ValueProperty(const QString &name,
                  const QVariant &value,
                  EditorFactory *editorFactory,
                  QObject *parent = nullptr);

    QVariant value() const override { return m_value; }
    void setValue(const QVariant &value) override;

private:
    QVariant m_value;
};

/**
 * A property that wraps a value of a QObject property and uses an editor
 * factory to create its editor.
 *
 * The property does not take ownership of the editor factory.
 */
class QObjectProperty : public AbstractProperty
{
    Q_OBJECT

public:
    QObjectProperty(QObject *object,
                    QMetaProperty property,
                    const QString &displayName,
                    EditorFactory *editorFactory,
                    QObject *parent = nullptr);

    QVariant value() const override;
    void setValue(const QVariant &value) override;

private:
    QObject *m_object;
    QMetaProperty m_property;
};

/**
 * An editor factory that selects the appropriate editor factory based on the
 * type of the property value.
 *
 * todo: rename to VariantEditorFactory when the old one is removed
 */
class ValueTypeEditorFactory : public EditorFactory
{
public:
    ValueTypeEditorFactory();

    /**
     * Register an editor factory for a given type.
     *
     * When there is already an editor factory registered for the given type,
     * it will be replaced.
     */
    void registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory);

    /**
     * Creates a property that wraps a QObject property and will use the editor
     * factory registered for the type of the value.
     */
    QObjectProperty *createQObjectProperty(QObject *qObject,
                                           const char *name,
                                           const QString &displayName = {});

    /**
     * Creates a property with the given name and value. The property will use
     * the editor factory registered for the type of the value.
     */
    ValueProperty *createProperty(const QString &name, const QVariant &value);

    /**
     * Creates a property with the given name and get/set functions. The
     * property will use the editor factory registered for the type of the
     * value.
     */
    AbstractProperty *createProperty(const QString &name,
                                     std::function<QVariant()> get,
                                     std::function<void(const QVariant&)> set);

    QWidget *createEditor(Property *property, QWidget *parent) override;

private:
    std::unordered_map<int, std::unique_ptr<EditorFactory>> m_factories;
};

/**
 * A property that wraps an enum value and uses an editor factory to create
 * its editor.
 */
class EnumProperty : public AbstractProperty
{
    Q_OBJECT

public:
    EnumProperty(const QString &name,
                 QObject *parent = nullptr);

    void setEnumNames(const QStringList &enumNames);
    void setEnumValues(const QList<int> &enumValues);

private:
    EnumEditorFactory m_editorFactory;
};


class VariantEditor : public QScrollArea
{
    Q_OBJECT

public:
    VariantEditor(QWidget *parent = nullptr);

    void clear();
    void addHeader(const QString &text);
    void addSeparator();
    void addProperty(Property *property);
    // void addValue(const QVariant &value);

private:
    QWidget *createEditor(Property *property);

    enum Column {
        LeftSpacing,
        LabelColumn,
        MiddleSpacing,
        WidgetColumn,
        RightSpacing,
        ColumnCount,
    };

    QWidget *m_widget;
    QGridLayout *m_gridLayout;
    int m_rowIndex = 0;
};

} // namespace Tiled
